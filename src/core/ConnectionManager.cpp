#include "core/ConnectionManager.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <sstream>

ConnectionManager::ConnectionManager(RequestHandler& requestHandler, CgiHandler& cgiHandler)
    : _requestHandler(requestHandler), _cgiHandler(cgiHandler)
{
}

ConnectionManager::~ConnectionManager()
{
    // Note: Clients should be cleaned up via closeAllConnections or in destructor
    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        delete it->second;
    }
}

void ConnectionManager::addServerConfig(int serverFd, const ServerConfig& config)
{
    _serverConfigs[serverFd] = config;
}

ClientConnection* ConnectionManager::getClient(int fd)
{
    if (_clients.find(fd) != _clients.end())
        return _clients[fd];
    return NULL;
}

bool ConnectionManager::hasClient(int fd) const
{
    return _clients.find(fd) != _clients.end();
}

void ConnectionManager::acceptNewConnection(int serverFd, ServerSocket* server, Poller& poller)
{
    int clientFd = server->acceptClient();
    if (clientFd < 0)
        return;

    size_t maxBodySize = MAX_BODY_SIZE;
    if (_serverConfigs.find(serverFd) != _serverConfigs.end())
        maxBodySize = _serverConfigs[serverFd].getClientMaxBodySize();

    ClientConnection* client = new ClientConnection(clientFd, maxBodySize);
    _clients[clientFd] = client;
    _clientToServer[clientFd] = serverFd;

    // Add client to poller (watch for EPOLLIN - incoming data)
    // EPOLLIN value is defined in CgiHandler but standard in sys/epoll.h or Poller.hpp
    if (!poller.addFd(clientFd, EPOLLIN))
    {
        Logger::error(Logger::fdMsg("Failed to add client to poller", clientFd));
        _clients.erase(clientFd);
        _clientToServer.erase(clientFd);
        delete client;
        close(clientFd);
        return;
    }

    Logger::info(Logger::connMsg("New client connected", clientFd));
}

void ConnectionManager::handleRead(int clientFd, Poller& poller)
{
    char buffer[BUFFER_SIZE];
    int n = recv(clientFd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        if (n == 0)
            Logger::debug(Logger::connMsg("Client closed connection", clientFd));
        else
            Logger::warn(Logger::connMsg(std::string("Client read error: ") + std::strerror(errno), clientFd));

        handleDisconnect(clientFd, poller);
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
        
        // Default config if not found (should not happen if addServerConfig used correctly)
        ServerConfig defaultConfig;
        const ServerConfig &config = (_serverConfigs.find(serverFd) != _serverConfigs.end()) 
                                     ? _serverConfigs[serverFd] 
                                     : defaultConfig;

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

        // Delegate to RequestHandler
        HttpResponse response = _requestHandler.handleRequest(request, location);

        if (response.isCgi())
        {
             _cgiHandler.startCgi(client, request, response, poller);
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
            poller.modifyFd(clientFd, EPOLLOUT);
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
        poller.modifyFd(clientFd, EPOLLOUT);
    }
    // else: Still parsing, wait for more data
}

void ConnectionManager::handleWrite(int clientFd, Poller& poller)
{
    if (_clients.find(clientFd) == _clients.end()) return;
    
    ClientConnection *c = _clients[clientFd];
    const std::string &data = c->getWriteBuffer();

    ssize_t bytes = send(clientFd, data.c_str(), data.size(), 0);
    if (bytes <= 0)
    {
        Logger::warn(Logger::connMsg("Client write failed", clientFd));
        handleDisconnect(clientFd, poller);
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
        handleDisconnect(clientFd, poller);
        return;
    }

    c->setState(READING);

    // Change back to monitor for read events
    poller.modifyFd(clientFd, EPOLLIN);
}

void ConnectionManager::handleDisconnect(int fd, Poller& poller)
{
    Logger::info(Logger::connMsg("Client disconnected", fd));

    if (_clients.count(fd) == 0)
        return;

    poller.removeFd(fd);

    ClientConnection *client = _clients[fd];

    // Cleanup CGI if active
    if (client->getCgiState().active)
    {
        _cgiHandler.cleanupCgi(client, poller);
    }

    _clients.erase(fd);
    _clientToServer.erase(fd);
    delete client;
}

void ConnectionManager::closeAllConnections(Poller& poller)
{
    std::vector<int> fds;
    for (std::map<int, ClientConnection*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        fds.push_back(it->first);
    }
    
    for (size_t i = 0; i < fds.size(); ++i)
    {
        handleDisconnect(fds[i], poller);
    }
}
