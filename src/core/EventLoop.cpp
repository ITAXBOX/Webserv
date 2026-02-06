#include "core/EventLoop.hpp"

EventLoop::EventLoop()
    : _running(true),
      _requestHandler(new RequestHandler()),
      _connManager(*_requestHandler, _cgiHandler)
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
    // ConnectionManager owns clients and cleans them up
    // However, it doesn't own ServerSockets, we do.

    // Cleanup servers
    for (std::map<int, ServerSocket *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        delete it->second;
    }
    _servers.clear();

    delete _requestHandler;
    Logger::debug("EventLoop destroyed");
}

void EventLoop::addServer(ServerSocket *server, const ServerConfig &config)
{
    _servers[server->getFd()] = server;
    _connManager.addServerConfig(server->getFd(), config);

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

            // 1. Handle Server Sockets (New Connections)
            if (_servers.count(ev.fd))
            {
                if (ev.readable)
                    _connManager.acceptNewConnection(ev.fd, _servers[ev.fd], _poller);
                continue;
            }

            // 2. Handle Client Connections
            if (_connManager.hasClient(ev.fd))
            {
                if (ev.readable)
                    _connManager.handleRead(ev.fd, _poller);

                // Note: handleRead might have closed the connection or started CGI
                // so we check hasClient again if needed
                if (_connManager.hasClient(ev.fd) && ev.writable)
                    _connManager.handleWrite(ev.fd, _poller);

                if ((ev.error || ev.hangup) && !_connManager.hasClient(ev.fd))
                {
                    // Already handled
                }
                else if ((ev.error || ev.hangup) && !ev.readable)
                {
                    _connManager.handleDisconnect(ev.fd, _poller);
                }

                continue;
            }

            // 3. Handle CGI Pipes
            if (_cgiHandler.hasCgiPipe(ev.fd))
            {
                int clientFd = _cgiHandler.getClientFd(ev.fd);
                ClientConnection *client = _connManager.getClient(clientFd);

                if (client)
                {
                    if (ev.readable)
                        _cgiHandler.handleCgiRead(ev.fd, client, _poller);
                    else if (ev.writable)
                        _cgiHandler.handleCgiWrite(ev.fd, client, _poller);
                    else if ((ev.error || ev.hangup))
                        _cgiHandler.handleCgiHangup(ev.fd, client, _poller);
                }
            }
        }
    }

    Logger::info("Event loop ended.");
}

void EventLoop::stop()
{
    _running = false;
    Logger::info("Stopping server...");

    _connManager.closeAllConnections(_poller);

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
