#ifndef EVENTLOOP_HPP
# define EVENTLOOP_HPP

#include <map>
#include <signal.h>
#include "core/Poller.hpp"
#include "core/ServerSocket.hpp"
#include "core/ClientConnection.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

class EventLoop
{
private:
    bool _running;
    Poller _poller;  // Using Poller instead of raw poll()
    std::map<int, ServerSocket*> _servers;
    std::map<int, ClientConnection*> _clients;

    void handleNewConnection(int serverFd);
    void handleClientRead(int clientFd);
    void handleClientWrite(int clientFd);
    void handleClientDisconnect(int fd);
    
    // Request handling
    HttpResponse handleRequest(const HttpRequest& request);

public:
    EventLoop();
    ~EventLoop();

    void addServer(ServerSocket* server);
    void run();
    void stop();
};

#endif