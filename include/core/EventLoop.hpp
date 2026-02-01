#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include <signal.h>
#include <map>
#include "core/Poller.hpp"
#include "core/ServerSocket.hpp"
#include "core/ClientConnection.hpp"
#include "core/ConnectionManager.hpp"
#include "core/CgiHandler.hpp"
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
    RequestHandler* _requestHandler;                   // Strategy pattern handler
    CgiHandler _cgiHandler;
    ConnectionManager _connManager;

public:
    EventLoop();
    ~EventLoop();

    void addServer(ServerSocket* server, const ServerConfig& config);
    void run();
    void stop();
};

#endif // EVENTLOOP_HPP