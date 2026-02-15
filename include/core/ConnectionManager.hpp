#ifndef CONNECTIONMANAGER_HPP
#define CONNECTIONMANAGER_HPP

#include "app/RequestHandler.hpp"
#include "config/ServerConfig.hpp"
#include "core/ClientConnection.hpp"
#include "core/ServerSocket.hpp"
#include "core/CgiHandler.hpp"
#include "core/Poller.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sstream>
#include <map>

class ConnectionManager
{
public:
    ConnectionManager(RequestHandler &requestHandler, CgiHandler &cgiHandler);
    ~ConnectionManager();

    // Configuration
    void addServerConfig(int serverFd, const ServerConfig &config);

    // Connection Lifecycle
    void acceptNewConnection(int serverFd, ServerSocket *server, Poller &poller);
    void handleRead(int clientFd, Poller &poller);
    void handleWrite(int clientFd, Poller &poller);
    void handleDisconnect(int clientFd, Poller &poller);

    // Accessors
    ClientConnection *getClient(int fd);
    bool hasClient(int fd) const;
    void closeAllConnections(Poller &poller);

    void checkCgiTimeouts(Poller &poller);

private:
    std::map<int, ClientConnection *> _clients;
    std::map<int, int> _clientToServer;         // Map client FD to server FD
    std::map<int, ServerConfig> _serverConfigs; // serverFd -> config

    RequestHandler &_requestHandler;
    CgiHandler &_cgiHandler;

    // Request processing helpers
    const ServerConfig &resolveConfig(int clientFd);
    LocationConfig     resolveLocation(const HttpRequest &request, const ServerConfig &config);
    void               processRequest(int clientFd, ClientConnection *client, Poller &poller);
    void               processParseError(int clientFd, ClientConnection *client, Poller &poller);

    void sendResponse(ClientConnection *client, HttpResponse &response, Poller &poller);
    void applyCustomErrorPage(HttpResponse &response, const ServerConfig &config);
};

#endif
