#include "core/EventLoop.hpp"
#include "utils/Logger.hpp"
#include "utils/defines.hpp"
#include "app/CgiExecutor.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
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
    for (std::map<int, ClientConnection *>::iterator it = _clients.begin(); it != _clients.end(); it++)
        delete it->second;

    delete _requestHandler;
    Logger::debug("EventLoop destroyed");
}

void EventLoop::addServer(ServerSocket *server, const ServerConfig &config)
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
        int n = _poller.wait(-1); // -1 = infinite timeout

        if (n < 0)
        {
            Logger::error("Poller wait failed");
            break;
        }

        if (n == 0)
            continue; // No events (shouldn't happen with infinite timeout)

        // Get events from poller
        const std::vector<PollEvent> &events = _poller.getEvents();

        // Process each event
        for (size_t i = 0; i < events.size(); i++)
        {
            const PollEvent &ev = events[i];

            // 1. Handle Readable (Data available or EOF) from CLIENTS
            if (ev.readable)
            {
                if (_servers.count(ev.fd))
                    handleNewConnection(ev.fd);
                else if (_clients.count(ev.fd))
                    handleClientRead(ev.fd);
                // CGI handled below
            }

            // 2. Handle Writable from CLIENTS
            // Only if connection is still valid (handleClientRead might have closed it)
            if (ev.writable && _clients.count(ev.fd))
            {
                handleClientWrite(ev.fd);
            }

            // 3. Handle CGI (Special case: Pipes)
            // CGI pipes are in _cgiToClient but NOT in _clients (keys)
            // But _clientToServer logic uses same FD space? No.
            // We need to check exact FD match for CGI pipes.
            if (_cgiToClient.count(ev.fd))
            {
                if (ev.readable)
                    handleCgiRead(ev.fd);
                if (ev.writable)
                    handleCgiWrite(ev.fd);
                if ((ev.error || ev.hangup) && !ev.readable)
                    handleCgiHangup(ev.fd);
            }

            // 4. Handle Error/Hangup for CLIENTS
            // Use else if to avoid double handling if read/write already handled separation
            // But if readable was handled, it might have closed the connection.
            if ((ev.error || ev.hangup) && _clients.count(ev.fd) && !ev.readable)
            {
                Logger::warn(Logger::fdMsg("Connection error/hangup detected", ev.fd));
                handleClientDisconnect(ev.fd);
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
    for (std::map<int, ClientConnection *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
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
    for (std::map<int, ServerSocket *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
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

    size_t maxBodySize = MAX_BODY_SIZE;
    if (_serverConfigs.find(serverFd) != _serverConfigs.end())
        maxBodySize = _serverConfigs[serverFd].getClientMaxBodySize();

    _clients[clientFd] = new ClientConnection(clientFd, maxBodySize);
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

    ClientConnection *client = _clients[clientFd];

    // Feed the data chunk to the HTTP parser
    std::string chunk(buffer, n);
    client->getParser().parse(chunk);

    // Check if parsing is complete
    if (client->getParser().isComplete())
    {
        Logger::info(Logger::connMsg("HTTP request parsing complete", clientFd));

        // Get the parsed request
        HttpRequest &request = client->getParser().getRequest();

        // Resolve Config
        int serverFd = _clientToServer[clientFd];
        const ServerConfig &config = _serverConfigs[serverFd];

        // Resolve Location
        const LocationConfig *locationPtr = config.matchLocation(request.getUri());

        // Create an effective location config that inherits from server config
        LocationConfig location;

        if (locationPtr)
        {
            location = *locationPtr;
        }
        else
        {
            location = LocationConfig("/");
        }

        // Inherit root if not specified in location
        if (location.getRoot().empty())
        {
            location.setRoot(config.getRoot());
        }

        // Inherit index if not specified in location
        if (location.getIndex().empty())
        {
            const std::vector<std::string> &serverIndices = config.getIndex();
            for (size_t i = 0; i < serverIndices.size(); ++i)
            {
                location.addIndex(serverIndices[i]);
            }
        }

        // Delegate to RequestHandler (Strategy Pattern)
        HttpResponse response = _requestHandler->handleRequest(
            request,
            location);

        if (response.isCgi())
        {
            startCgi(clientFd, client, request, response);
        }
        else
        {
            // Check if we should close the connection after this response
            if (response.getHeader("Connection") == "close")
                client->setShouldClose(true);

            client->appendToWriteBuffer(response.build());
            client->setState(WRITING);

            // Reset parser for next request (keep-alive support)
            client->getParser().reset();

            // Change to monitor for write events
            _poller.modifyFd(clientFd, EPOLLOUT);
        }
    }
    else if (client->getParser().hasError())
    {
        Logger::error(Logger::connMsg("HTTP parsing error: " + client->getParser().getErrorMessage(), clientFd));

        // Determine Error Code
        int code = HTTP_BAD_REQUEST;
        std::string msg = "Bad Request";
        if (client->getParser().getErrorMessage() == "Payload Too Large")
        {
            code = 413;
            msg = "Payload Too Large";
        }

        // Send Error Response
        HttpResponse response = StatusCodes::createErrorResponse(code, msg);
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
    const std::string &data = c->getWriteBuffer();

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

    // Close connection if requested (e.g. after DELETE or Connection: close)
    if (c->shouldClose())
    {
        Logger::info(Logger::connMsg("Closing connection as requested", clientFd));
        handleClientDisconnect(clientFd);
        return;
    }

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

    ClientConnection *client = _clients[fd];

    // Cleanup CGI if active
    if (client->getCgiState().active)
    {
        CgiState &state = client->getCgiState();
        if (state.pipeIn[1] != -1)
        {
            _poller.removeFd(state.pipeIn[1]);
            _cgiToClient.erase(state.pipeIn[1]);
            close(state.pipeIn[1]);
        }
        if (state.pipeOut[0] != -1)
        {
            _poller.removeFd(state.pipeOut[0]);
            _cgiToClient.erase(state.pipeOut[0]);
            close(state.pipeOut[0]);
        }
        kill(state.pid, SIGKILL);
        waitpid(state.pid, NULL, WNOHANG);
    }

    _clients.erase(fd);
    _clientToServer.erase(fd);
    delete client;
}

void EventLoop::startCgi(int clientFd, ClientConnection *client, const HttpRequest &request, const HttpResponse &response)
{
    // Reset CGI state for new execution
    client->getCgiState() = CgiState();

    CgiExecutor executor;
    executor.start(request, response.getCgiScriptPath(), response.getCgiInterpreterPath(), client->getCgiState());

    if (!client->getCgiState().active)
    {
        Logger::error("Failed to start CGI");
        client->appendToWriteBuffer(StatusCodes::createErrorResponse(500, "CGI Start Failed").build());
        client->setState(WRITING);
        _poller.modifyFd(clientFd, EPOLLOUT);
        return;
    }

    CgiState &state = client->getCgiState();

    // Register pipeIn[1] for Writing (sending body to child)
    if (state.pipeIn[1] != -1)
    {
        if (!_poller.addFd(state.pipeIn[1], EPOLLOUT))
        {
            Logger::error("Failed to add CGI input pipe to poller");
            // Handle error cleanup
        }
        else
        {
            _cgiToClient[state.pipeIn[1]] = clientFd;
        }
    }

    // Register pipeOut[0] for Reading (reading response from child)
    if (state.pipeOut[0] != -1)
    {
        if (!_poller.addFd(state.pipeOut[0], EPOLLIN))
        {
            Logger::error("Failed to add CGI output pipe to poller");
        }
        else
        {
            _cgiToClient[state.pipeOut[0]] = clientFd;
        }
    }

    client->setState(CGI_ACTIVE);
    Logger::info(Logger::connMsg("CGI Started asynchronously", clientFd));
}

void EventLoop::handleCgiWrite(int pipeFd)
{
    if (_cgiToClient.find(pipeFd) == _cgiToClient.end())
        return;
    int clientFd = _cgiToClient[pipeFd];
    ClientConnection *client = _clients[clientFd];
    CgiState &state = client->getCgiState();

    if (state.pipeIn[1] != pipeFd)
        return;

    const std::string &body = state.requestBody;
    if (!body.empty())
    {
        ssize_t bytes = write(pipeFd, body.c_str(), body.size());
        if (bytes > 0)
        {
            state.requestBody.erase(0, bytes);
        }
    }

    if (state.requestBody.empty())
    {
        _poller.removeFd(pipeFd);
        _cgiToClient.erase(pipeFd);
        close(state.pipeIn[1]);
        state.pipeIn[1] = -1;
        Logger::debug("CGI input closed");
    }
}

void EventLoop::handleCgiRead(int pipeFd)
{
    if (_cgiToClient.find(pipeFd) == _cgiToClient.end())
        return;
    int clientFd = _cgiToClient[pipeFd];
    ClientConnection *client = _clients[clientFd];
    CgiState &state = client->getCgiState();

    if (state.pipeOut[0] != pipeFd)
        return;

    char buffer[4096];
    ssize_t bytes = read(pipeFd, buffer, sizeof(buffer));

    if (bytes > 0)
    {
        state.responseBuffer.append(buffer, bytes);
    }
    else
    {
        // EOF or Error
        handleCgiHangup(pipeFd);
    }
}

void EventLoop::handleCgiHangup(int pipeFd)
{
    if (_cgiToClient.find(pipeFd) == _cgiToClient.end())
        return;

    int clientFd = _cgiToClient[pipeFd];
    ClientConnection *client = _clients[clientFd];
    CgiState &state = client->getCgiState();

    // Remove pipe from poller
    _poller.removeFd(pipeFd);
    _cgiToClient.erase(pipeFd);

    if (state.pipeOut[0] == pipeFd)
    {
        close(state.pipeOut[0]);
        state.pipeOut[0] = -1;
    }
    if (state.pipeIn[1] == pipeFd)
    {
        close(state.pipeIn[1]);
        state.pipeIn[1] = -1;
    }

    // If output pipe is closed, we assume CGI is done sending data
    if (state.pipeOut[0] == -1)
    {
        state.active = false;

        // Ensure child process is terminated and reaped to avoid zombies
        kill(state.pid, SIGKILL);
        // Small chance of race condition where process is not yet zombie,
        // but we can't block. SIGKILL ensures it dies.
        waitpid(state.pid, NULL, WNOHANG);

        Logger::info("CGI Finished, processing response");

        // Process Response
        HttpResponse response;
        response.setStatus(200, "OK");

        std::string &raw = state.responseBuffer;
        // Support both CRLF and LF for headers
        size_t headerEnd = raw.find("\r\n\r\n");
        size_t bodyStart = 0;

        if (headerEnd != std::string::npos)
        {
            bodyStart = headerEnd + 4;
        }
        else
        {
            headerEnd = raw.find("\n\n");
            if (headerEnd != std::string::npos)
            {
                bodyStart = headerEnd + 2;
            }
        }

        if (headerEnd != std::string::npos)
        {
            std::string headers = raw.substr(0, headerEnd);
            std::string body = raw.substr(bodyStart);

            std::istringstream stream(headers);
            std::string line;
            while (std::getline(stream, line))
            {
                if (!line.empty() && line[line.size() - 1] == '\r')
                    line.resize(line.size() - 1);

                size_t colon = line.find(':');
                if (colon != std::string::npos)
                {
                    std::string key = line.substr(0, colon);
                    std::string val = line.substr(colon + 1);
                    size_t first = val.find_first_not_of(" \t");
                    if (first != std::string::npos)
                        val = val.substr(first);

                    if (key == "Status")
                    {
                        size_t sp = val.find(' ');
                        if (sp != std::string::npos)
                            response.setStatus(std::atoi(val.c_str()), val.substr(sp + 1));
                        else
                            response.setStatus(std::atoi(val.c_str()), "OK");
                    }
                    else
                    {
                        response.addHeader(key, val);
                    }
                }
            }
            response.setBody(body);

            // Ensure Content-Length is set to prevent client hanging
            std::ostringstream ss;
            ss << body.size();
            response.addHeader("Content-Length", ss.str());
        }
        else
        {
            response.setBody(raw);
            std::ostringstream ss;
            ss << raw.size();
            response.addHeader("Content-Length", ss.str());
            response.addHeader("Content-Type", "text/plain"); // Default fallback
        }

        client->appendToWriteBuffer(response.build());
        client->setState(WRITING);
        client->getParser().reset();
        _poller.modifyFd(clientFd, EPOLLOUT);
    }
}
