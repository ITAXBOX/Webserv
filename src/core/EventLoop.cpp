#include "core/EventLoop.hpp"
#include "utils/Logger.hpp"
#include "utils/defines.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "http/HttpResponse.hpp"
#include "utils/StatusCodes.hpp"

static EventLoop* g_loop = NULL;

extern "C" void signalHandler(int)
{
    if (g_loop)
    {
        Logger::info("Received interrupt signal (Ctrl+C)");
        g_loop->stop();
    }
}

EventLoop::EventLoop() : _running(true)
{
    g_loop = this;
    signal(SIGINT, signalHandler);  // Ctrl+C for graceful shutdown
    signal(SIGTSTP, SIG_IGN);       // Ignore Ctrl+Z to prevent suspension
    
    if (!_poller.isValid())
    {
        Logger::error("Failed to create Poller (epoll)");
        _running = false;
    }
    
    Logger::debug("EventLoop initialized with Poller");
}

EventLoop::~EventLoop()
{
    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); it++)
        delete it->second;
    Logger::debug("EventLoop destroyed");
}

void EventLoop::addServer(ServerSocket* server)
{
    _servers[server->getFd()] = server;
    
    // Add server socket to poller (watch for EPOLLIN - incoming connections)
    if (!_poller.addFd(server->getFd(), EPOLLIN))
    {
        Logger::error(Logger::fdMsg("Failed to add server to poller", server->getFd()));
        return;
    }
    
    Logger::debug(Logger::fdMsg("Server added to event loop", server->getFd()));
}

void EventLoop::run()
{
    Logger::info("Event loop started with epoll. Press Ctrl+C to stop.");
    
    while (_running)
    {
        // Wait for events using Poller (epoll-based)
        int n = _poller.wait(-1);  // -1 = infinite timeout
        
        if (n < 0)
        {
            Logger::error("Poller wait failed");
            break;
        }
        
        if (n == 0)
            continue;  // No events (shouldn't happen with infinite timeout)
        
        // Get events from poller
        const std::vector<PollEvent>& events = _poller.getEvents();
        
        // Process each event
        for (size_t i = 0; i < events.size(); i++)
        {
            const PollEvent& ev = events[i];
            
            // Handle errors/hangups first
            if (ev.error || ev.hangup)
            {
                Logger::warn(Logger::fdMsg("Connection error/hangup detected", ev.fd));
                if (_clients.count(ev.fd))
                    handleClientDisconnect(ev.fd);
                continue;
            }
            
            // Handle readable events
            // we only read from servers (new connections) and clients (data)
            if (ev.readable)
            {
                if (_servers.count(ev.fd))
                    handleNewConnection(ev.fd);
                else if (_clients.count(ev.fd))
                    handleClientRead(ev.fd);
            }
            
            // Handle writable events
            // we only write to clients
            if (ev.writable)
            {
                if (_clients.count(ev.fd))
                    handleClientWrite(ev.fd);
            }
        }
    }
    
    Logger::info("Event loop ended.");
}

void EventLoop::stop()
{
    _running = false;
    Logger::info("Stopping server...");

    // Cleanup clients first
    Logger::debug("Cleaning up client connections");
    std::vector<int> clientFds;
    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        clientFds.push_back(it->first);
    
    for (size_t i = 0; i < clientFds.size(); i++)
    {
        int fd = clientFds[i];
        _poller.removeFd(fd);
        delete _clients[fd];
    }
    _clients.clear();

    // Cleanup servers
    Logger::debug("Cleaning up server sockets");
    std::vector<int> serverFds;
    for (std::map<int, ServerSocket*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
        serverFds.push_back(it->first);
    
    for (size_t i = 0; i < serverFds.size(); i++)
    {
        int fd = serverFds[i];
        _poller.removeFd(fd);
        delete _servers[fd];
    }
    _servers.clear();
    
    Logger::info("Server stopped successfully");
}

// Handle new incoming connection on server socket
void EventLoop::handleNewConnection(int serverFd)
{
    int clientFd = _servers[serverFd]->acceptClient();
    if (clientFd < 0)
        return;
    
    // Note: ServerSocket already sets non-blocking in acceptClient()
    _clients[clientFd] = new ClientConnection(clientFd);

    // Add client to poller (watch for EPOLLIN - incoming data)
    if (!_poller.addFd(clientFd, EPOLLIN))
    {
        Logger::error(Logger::fdMsg("Failed to add client to poller", clientFd));
        delete _clients[clientFd];
        _clients.erase(clientFd);
        close(clientFd);
        return;
    }

    Logger::info(Logger::connMsg("New client connected", clientFd));
}

// Handle reading data from client
// TCP read event
void EventLoop::handleClientRead(int clientFd)
{
    char buffer[BUFFER_SIZE];
    int n = recv(clientFd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        // n == 0 means graceful shutdown (normal), n < 0 means error
        if (n == 0)
            Logger::debug(Logger::connMsg("Client closed connection", clientFd));
        else
            Logger::warn(Logger::connMsg(std::string("Client read error: ") + std::strerror(errno), clientFd));
        
        handleClientDisconnect(clientFd);
        return;
    }
    
    std::ostringstream os;
    os << "Received " << n << " bytes from client";
    Logger::debug(Logger::connMsg(os.str(), clientFd));

    ClientConnection* client = _clients[clientFd];
    
    // Feed the data chunk to the HTTP parser
    std::string chunk(buffer, n);
    client->getParser().parse(chunk);
    
    // Check if parsing is complete
    if (client->getParser().isComplete())
    {
        Logger::info(Logger::connMsg("HTTP request parsing complete", clientFd));
        
        // Get the parsed request
        HttpRequest& request = client->getParser().getRequest();
        
        // Process the request and generate response
        HttpResponse response = handleRequest(request);
        client->appendToWriteBuffer(response.build());
        client->setState(WRITING);
        
        // Reset parser for next request (keep-alive support)
        client->getParser().reset();
        
        // Change to monitor for write events
        _poller.modifyFd(clientFd, EPOLLOUT);
    }
    else if (client->getParser().hasError())
    {
        Logger::error(Logger::connMsg("HTTP parsing error: " + client->getParser().getErrorMessage(), clientFd));
        
        // Send 400 Bad Request
        HttpResponse response = StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
        client->appendToWriteBuffer(response.build());
        client->setState(WRITING);
        
        // Change to monitor for write events
        _poller.modifyFd(clientFd, EPOLLOUT);
    }
    // else: Still parsing, wait for more data (do nothing)
}

// Handle writing data to client
// TCP write event
void EventLoop::handleClientWrite(int clientFd)
{
    ClientConnection *c = _clients[clientFd];
    const std::string& data = c->getWriteBuffer();

    ssize_t bytes = send(clientFd, data.c_str(), data.size(), 0);
    if (bytes <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        Logger::warn(Logger::connMsg("Client write failed", clientFd));
        handleClientDisconnect(clientFd);
        return;
    }
    
    std::ostringstream os;
    os << "Sent " << bytes << " bytes to client";
    Logger::debug(Logger::connMsg(os.str(), clientFd));
    
    if ((size_t)bytes < data.size())
    {
        c->getWriteBuffer().erase(0, bytes);
        Logger::debug(Logger::connMsg("Partial write, data remaining", clientFd));
        return;
    }
    c->clearWriteBuffer();
    c->setState(READING);

    // Change back to monitor for read events
    _poller.modifyFd(clientFd, EPOLLIN);
}

// Handle client disconnection and cleanup
void EventLoop::handleClientDisconnect(int fd)
{
    Logger::info(Logger::connMsg("Client disconnected", fd));
    
    // Check if client exists
    if (_clients.count(fd) == 0)
        return;
    
    // Remove from poller first (before closing fd)
    _poller.removeFd(fd);
    
    // Clean up client connection (this closes the fd)
    ClientConnection* client = _clients[fd];
    _clients.erase(fd);
    delete client;
}

// Handle HTTP request and generate response
HttpResponse EventLoop::handleRequest(const HttpRequest& request)
{
    // Log the request
    std::ostringstream os;
    os << request.getMethodString() << " " << request.getUri() << " " << request.getVersion();
    Logger::info("Processing request: " + os.str());
    
    // Get the HTTP method
    HttpMethod method = request.getMethod();
    
    // Only support GET and HEAD for now
    if (method != HTTP_GET && method != HTTP_HEAD)
    {
        Logger::warn("Method not allowed: " + request.getMethodString());
        return StatusCodes::createErrorResponse(HTTP_METHOD_NOT_ALLOWED, "Method Not Allowed");
    }
    
    // Get the requested URI
    std::string uri = request.getUri();
    
    // Remove query string if present (e.g., /index.html?foo=bar -> /index.html)
    size_t queryPos = uri.find('?');
    if (queryPos != std::string::npos)
        uri = uri.substr(0, queryPos);
    
    // Security: Detect and block suspicious path patterns
    // Return 404 instead of 403 to prevent information disclosure
    // We did this to prevent Information Disclosure through Error Messages
    // For example, if a user tries to access a restricted directory,
    // we dont want to give the attacker clues about the server structure
    // like whether the directory exists or forbidden
    // To prevent attackers from inferring server structure
    // Noticed during security reviews
    if (uri.find("..") != std::string::npos)
    {
        Logger::warn("Directory traversal attempt detected: " + uri);
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }
    if (uri.find("//") != std::string::npos)
    {
        Logger::warn("Double slash detected in URI: " + uri);
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }
    if (uri.find("\\") != std::string::npos)
    {
        Logger::warn("Backslash detected in URI: " + uri);
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }
    // Prevent null byte injection
    if (uri.find('\0') != std::string::npos)
    {
        Logger::warn("Null byte detected in URI");
        return StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
    }
    
    // Build file path (default root is tests/)
    std::string filePath = DEFAULT_ROOT;
    if (uri == "/" || uri.empty())
        filePath += "/" + std::string(DEFAULT_INDEX);  // Default file
    else
        filePath += uri;
    
    Logger::debug("Serving file: " + filePath);
    
    // Try to serve the file
    HttpResponse response = StatusCodes::createOkResponse(filePath);
    
    // For HEAD requests, clear the body (only send headers)
    if (method == HTTP_HEAD)
    {
        response.setBody("");
        Logger::debug("HEAD request - body cleared");
    }
    
    return response;
}
