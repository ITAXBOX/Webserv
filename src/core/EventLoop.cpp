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
#include "app/RequestHandler.hpp"

EventLoop::EventLoop() 
    : _running(true), _requestHandler(new RequestHandler())
{
    if (!_poller.isValid())
    {
        Logger::error("Failed to create Poller (epoll)");
        _running = false;
    }
    
    Logger::debug("EventLoop initialized with Poller and RequestHandler");
}

EventLoop::~EventLoop()
{
    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); it++)
        delete it->second;
    
    delete _requestHandler;
    Logger::debug("EventLoop destroyed");
}

void EventLoop::addServer(ServerSocket* server, const ServerConfig& config)
{
    _servers[server->getFd()] = server;
    _serverConfigs[server->getFd()] = config;
    
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
            if (ev.readable)
            {
                if (_servers.count(ev.fd))
                    handleNewConnection(ev.fd);
                else if (_clients.count(ev.fd))
                    handleClientRead(ev.fd);
            }
            
            // Handle writable events
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

void EventLoop::handleNewConnection(int serverFd)
{
    int clientFd = _servers[serverFd]->acceptClient();
    if (clientFd < 0)
        return;
    
    _clients[clientFd] = new ClientConnection(clientFd);
    _clientToServer[clientFd] = serverFd;

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

void EventLoop::handleClientRead(int clientFd)
{
    char buffer[BUFFER_SIZE];
    int n = recv(clientFd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {   
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
        
        // Resolve Config
        int serverFd = _clientToServer[clientFd];
        const ServerConfig &config = _serverConfigs[serverFd];
        
        // Resolve Location
        const LocationConfig *location = config.matchLocation(request.getUri());
        
        static LocationConfig defaultLocation("/");
        if (location == NULL) {
             defaultLocation.setRoot(config.getRoot());
             if (!config.getIndex().empty()) defaultLocation.addIndex(config.getIndex()[0]);
             location = &defaultLocation;
        }

        // Delegate to RequestHandler (Strategy Pattern)
        HttpResponse response = _requestHandler->handleRequest(
            request, 
            *location
        );
        
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
    // else: Still parsing, wait for more data
}

void EventLoop::handleClientWrite(int clientFd)
{
    ClientConnection *c = _clients[clientFd];
    const std::string& data = c->getWriteBuffer();

    ssize_t bytes = send(clientFd, data.c_str(), data.size(), 0);
    if (bytes <= 0)
    {
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

void EventLoop::handleClientDisconnect(int fd)
{
    Logger::info(Logger::connMsg("Client disconnected", fd));
    
    if (_clients.count(fd) == 0)
        return;
    
    _poller.removeFd(fd);
    
    ClientConnection* client = _clients[fd];
    _clients.erase(fd);
    _clientToServer.erase(fd);
    delete client;
}
