#include "core/EventLoop.hpp"
#include "utils/Logger.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sstream>

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
    signal(SIGINT, signalHandler);
    Logger::debug("EventLoop initialized");
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
    pollfd pfd = { server->getFd(), POLLIN, 0 };
    _fds.push_back(pfd);
    Logger::debug(Logger::fdMsg("Server added to event loop", server->getFd()));
}

void EventLoop::run()
{
    Logger::info("Event loop started. Press Ctrl+C to stop.");
    while (_running)
    {
        int ret = poll(_fds.data(), _fds.size(), -1);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                Logger::debug("poll() interrupted by signal");
                continue;
            }
            Logger::error(Logger::errnoMsg("poll() failed"));
            break;
        }
        
        Logger::debug("poll() returned, processing events");
        std::vector<int> toRemove;
        for (size_t i = 0; i < _fds.size(); i++)
        {
            pollfd &pfd = _fds[i];
            if (pfd.revents & POLLIN)
            {
                if (_servers.count(pfd.fd))
                    handleNewConnection(pfd.fd);
                else if (_clients.count(pfd.fd))
                    handleClientRead(pfd.fd);
            }
            else if (pfd.revents & POLLOUT)
                handleClientWrite(pfd.fd);
            else if (pfd.revents & (POLLERR | POLLHUP))
            {
                Logger::warn(Logger::fdMsg("Connection error/hangup detected", pfd.fd));
                toRemove.push_back(pfd.fd);
            }
        }

        for (size_t i = 0; i < toRemove.size(); i++)
            handleClientDisconnect(toRemove[i]);
    }
    Logger::info("Event loop ended.");
}

void EventLoop::stop()
{
    _running = false;
    Logger::info("Stopping server...");

    Logger::debug("Cleaning up client connections");
    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); it++)
        delete it->second;
    _clients.clear();

    Logger::debug("Cleaning up server sockets");
    for (std::map<int, ServerSocket*>::iterator it = _servers.begin(); it != _servers.end(); it++)
        delete it->second;
    _servers.clear();
    
    _fds.clear();
    Logger::info("Server stopped successfully");
}

void EventLoop::handleNewConnection(int serverFd)
{
    int clientFd = _servers[serverFd]->acceptClient();
    if (clientFd < 0)
        return;
    
    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    _clients[clientFd] = new ClientConnection(clientFd);

    pollfd newPfd = { clientFd, POLLIN, 0 };
    _fds.push_back(newPfd);

    Logger::info(Logger::connMsg("New client connected", clientFd));
}

void EventLoop::handleClientRead(int clientFd)
{
    char buffer[1024];
    int n = recv(clientFd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        Logger::warn(Logger::connMsg("Client read failed or closed connection", clientFd));
        handleClientDisconnect(clientFd);
        return;
    }
    
    std::string msg(buffer, n);
    std::ostringstream os;
    os << "Received " << n << " bytes from client";
    Logger::debug(Logger::connMsg(os.str(), clientFd));

    _clients[clientFd]->appendToWriteBuffer(msg);
    _clients[clientFd]->setState(WRITING);

    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == clientFd)
            _fds[i].events = POLLOUT;
    }
}

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

    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == clientFd)
            _fds[i].events = POLLIN;
    }
}

void EventLoop::handleClientDisconnect(int fd)
{
    Logger::info(Logger::connMsg("Client disconnected", fd));
    delete _clients[fd];
    _clients.erase(fd);

    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }
}
