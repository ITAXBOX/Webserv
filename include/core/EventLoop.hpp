#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <signal.h>
#include <map>
#include "core/Poller.hpp"
#include "core/ServerSocket.hpp"
#include "core/ClientConnection.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

// Forward declaration to avoid circular dependency
class RequestHandler;

class EventLoop
{
private:
    bool _running;
    Poller _poller;                                    // Using Poller instead of raw poll()
    std::map<int, ServerSocket*> _servers;
    std::map<int, ClientConnection*> _clients;
    RequestHandler* _requestHandler;                   // Strategy pattern handler

    void handleNewConnection(int serverFd);
    void handleClientRead(int clientFd);
    void handleClientWrite(int clientFd);
    void handleClientDisconnect(int fd);

public:
    EventLoop();
    ~EventLoop();

    void addServer(ServerSocket* server);
    void run();
    void stop();
};

#endif // EVENTLOOP_HPP