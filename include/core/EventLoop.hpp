#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <signal.h>
#include <map>
#include "core/Poller.hpp"
#include "core/ServerSocket.hpp"
#include "core/ClientConnection.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

#include "config/ServerConfig.hpp"

// Forward declaration to avoid circular dependency
class RequestHandler;

class EventLoop
{
private:
    bool _running;
    Poller _poller;                                    // Using Poller instead of raw poll()
    std::map<int, ServerSocket*> _servers;
    std::map<int, ServerConfig> _serverConfigs;        // Map server FD to its config
    std::map<int, ClientConnection*> _clients;
    std::map<int, int> _clientToServer;                // Map client FD to server FD (to look up config)
    RequestHandler* _requestHandler;                   // Strategy pattern handler

    void handleNewConnection(int serverFd);
    void handleClientRead(int clientFd);
    void handleClientWrite(int clientFd);
    void handleClientDisconnect(int fd);

public:
    EventLoop();
    ~EventLoop();

    void addServer(ServerSocket* server, const ServerConfig& config);
    void run();
    void stop();
};

#endif // EVENTLOOP_HPP