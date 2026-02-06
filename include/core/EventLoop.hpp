#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "config/ServerConfig.hpp"
#include "app/RequestHandler.hpp"
#include "core/Poller.hpp"
#include "core/ConnectionManager.hpp"
#include "core/ClientConnection.hpp"
#include "core/ServerSocket.hpp"
#include "http/HttpRequest.hpp"
#include "core/CgiHandler.hpp"
#include "http/HttpResponse.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include "utils/Logger.hpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <map>

// Forward declaration to avoid circular dependency
class RequestHandler;

class EventLoop
{
private:
    bool _running;
    Poller _poller; // Using Poller instead of raw poll()
    std::map<int, ServerSocket *> _servers;
    RequestHandler *_requestHandler; // Strategy pattern handler
    CgiHandler _cgiHandler;
    ConnectionManager _connManager;

public:
    EventLoop();
    ~EventLoop();

    void addServer(ServerSocket *server, const ServerConfig &config);
    void run();
    void stop();
};

#endif