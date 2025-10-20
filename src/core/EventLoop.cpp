#include "core/EventLoop.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <cstdio>

static EventLoop* g_loop = NULL;

extern "C" void signalHandler(int)
{
    if (g_loop)
        g_loop->stop();
}

EventLoop::EventLoop() : _running(true)
{
    g_loop = this;
    signal(SIGINT, signalHandler);
}

EventLoop::~EventLoop()
{
    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); it++)
        delete it->second;
}

void EventLoop::addServer(ServerSocket* server)
{
    _servers[server->fd()] = server;
    pollfd pfd = { server->fd(), POLLIN, 0 };
    _fds.push_back(pfd);
}

void EventLoop::run()
{
    std::cout << "Event loop started. Press Ctrl+C to stop." << std::endl;
    while (_running)
    {
        int ret = poll(_fds.data(), _fds.size(), -1);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            perror("poll");
            break;
        }
        
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
                toRemove.push_back(pfd.fd);
        }

        for (size_t i = 0; i < toRemove.size(); i++)
            handleClientDisconnect(toRemove[i]);
    }
    std::cout << "Event loop ended." << std::endl; 
}

void EventLoop::stop()
{
    _running = false;
    std::cout << "\nStopping server..." << std::endl;

    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); it++)
        delete it->second;
    _clients.clear();

    for (std::map<int, ServerSocket*>::iterator it = _servers.begin(); it != _servers.end(); it++)
        delete it->second;
    _servers.clear();
    
    _fds.clear();
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

    std::cout << "New client connected (fd= " << clientFd << ")" << std::endl;
}

void EventLoop::handleClientRead(int clientFd)
{
    char buffer[1024];
    int n = recv(clientFd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        handleClientDisconnect(clientFd);
        return;
    }
    
    std::string msg(buffer, n);
    std::cout << "[Client " << clientFd << "] " << msg << std::endl;

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
        handleClientDisconnect(clientFd);
        return;
    }
    if ((size_t)bytes < data.size())
    {
        c->getWriteBuffer().erase(0, bytes);
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
    std::cout << "Client disconnected (fd=" << fd << ")" << std::endl;
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
