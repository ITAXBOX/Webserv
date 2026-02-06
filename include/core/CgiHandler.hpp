#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "app/CgiExecutor.hpp"
#include "core/ClientConnection.hpp"
#include "core/Poller.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include <sys/wait.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <map>

class CgiHandler
{
public:
    CgiHandler();
    ~CgiHandler();

    // Start CGI process
    void startCgi(ClientConnection* client, const HttpRequest& request, const HttpResponse& response, Poller& poller);

    // Handle IO events
    void handleCgiRead(int pipeFd, ClientConnection* client, Poller& poller);
    void handleCgiWrite(int pipeFd, ClientConnection* client, Poller& poller);
    
    // Cleanup/Completion
    void handleCgiHangup(int pipeFd, ClientConnection* client, Poller& poller);
    void cleanupCgi(ClientConnection* client, Poller& poller);
    
    // Map Management
    int getClientFd(int pipeFd) const;
    bool hasCgiPipe(int pipeFd) const;

private:
    void processCgiResponse(ClientConnection* client);
    std::map<int, int> _pipeToClient; // pipeFd -> clientFd
};

#endif
