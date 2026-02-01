#include "core/CgiHandler.hpp"
#include "app/CgiExecutor.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/epoll.h> // For EPOLLIN/EPOLLOUT
#include <sstream>
#include <cstdlib>

CgiHandler::CgiHandler() {}

CgiHandler::~CgiHandler() {}

int CgiHandler::getClientFd(int pipeFd) const
{
    std::map<int, int>::const_iterator it = _pipeToClient.find(pipeFd);
    if (it != _pipeToClient.end())
        return it->second;
    return -1;
}

bool CgiHandler::hasCgiPipe(int pipeFd) const
{
    return _pipeToClient.count(pipeFd) > 0;
}

void CgiHandler::startCgi(ClientConnection* client, const HttpRequest& request, const HttpResponse& response, Poller& poller)
{
    // Reset CGI state for new execution
    client->getCgiState() = CgiState();

    CgiExecutor executor;
    executor.start(request, response.getCgiScriptPath(), response.getCgiInterpreterPath(), client->getCgiState());

    if (!client->getCgiState().active)
    {
        Logger::error("Failed to start CGI");
        client->appendToWriteBuffer(StatusCodes::createErrorResponse(500, "CGI Start Failed").build());
        client->setState(WRITING);
        // Assuming poller is accessible or we return status to update poller
        // Here we need to update poller outside or pass it in. We passed it in.
        poller.modifyFd(client->getFd(), EPOLLOUT);
        return;
    }

    CgiState &state = client->getCgiState();
    int clientFd = client->getFd();

    // Register pipeIn[1] for Writing (sending body to child)
    if (state.pipeIn[1] != -1)
    {
        if (!poller.addFd(state.pipeIn[1], EPOLLOUT))
        {
            Logger::error("Failed to add CGI input pipe to poller");
            cleanupCgi(client, poller);
            return;
        }
        else
        {
            _pipeToClient[state.pipeIn[1]] = clientFd;
        }
    }

    // Register pipeOut[0] for Reading (reading response from child)
    if (state.pipeOut[0] != -1)
    {
        if (!poller.addFd(state.pipeOut[0], EPOLLIN))
        {
            Logger::error("Failed to add CGI output pipe to poller");
             cleanupCgi(client, poller);
             return;
        }
        else
        {
            _pipeToClient[state.pipeOut[0]] = clientFd;
        }
    }

    client->setState(CGI_ACTIVE);
    Logger::info(Logger::connMsg("CGI Started asynchronously", clientFd));
}

void CgiHandler::cleanupCgi(ClientConnection* client, Poller& poller)
{
    CgiState &state = client->getCgiState();
    if (state.active)
    {
        if (state.pipeIn[1] != -1)
        {
            poller.removeFd(state.pipeIn[1]);
            _pipeToClient.erase(state.pipeIn[1]);
            close(state.pipeIn[1]);
            state.pipeIn[1] = -1;
        }
        if (state.pipeOut[0] != -1)
        {
            poller.removeFd(state.pipeOut[0]);
            _pipeToClient.erase(state.pipeOut[0]);
            close(state.pipeOut[0]);
            state.pipeOut[0] = -1;
        }
        kill(state.pid, SIGKILL);
        waitpid(state.pid, NULL, WNOHANG);
        state.active = false;
    }
}

void CgiHandler::handleCgiRead(int pipeFd, ClientConnection* client, Poller& poller)
{
    CgiState &state = client->getCgiState();

    if (state.pipeOut[0] != pipeFd)
        return;

    char buffer[4096];
    ssize_t bytes = read(pipeFd, buffer, sizeof(buffer));

    if (bytes > 0)
    {
        state.responseBuffer.append(buffer, bytes);
    }
    else
    {
        // EOF or Error
        handleCgiHangup(pipeFd, client, poller);
    }
}

void CgiHandler::handleCgiWrite(int pipeFd, ClientConnection* client, Poller& poller)
{
    CgiState &state = client->getCgiState();

    if (state.pipeIn[1] != pipeFd)
        return;

    const std::string &body = state.requestBody;
    if (!body.empty())
    {
        ssize_t bytes = write(pipeFd, body.c_str(), body.size());
        if (bytes > 0)
        {
            state.requestBody.erase(0, bytes);
        }
    }

    if (state.requestBody.empty())
    {
        poller.removeFd(pipeFd);
        _pipeToClient.erase(pipeFd);
        close(state.pipeIn[1]);
        state.pipeIn[1] = -1;
        Logger::debug("CGI input closed");
    }
}

void CgiHandler::handleCgiHangup(int pipeFd, ClientConnection* client, Poller& poller)
{
    CgiState &state = client->getCgiState();

    // Remove pipe from poller
    poller.removeFd(pipeFd);
    _pipeToClient.erase(pipeFd);

    if (state.pipeOut[0] == pipeFd)
    {
        close(state.pipeOut[0]);
        state.pipeOut[0] = -1;
    }
    if (state.pipeIn[1] == pipeFd)
    {
        close(state.pipeIn[1]);
        state.pipeIn[1] = -1;
    }

    // If output pipe is closed, we assume CGI is done sending data
    if (state.pipeOut[0] == -1)
    {
        state.active = false;

        // Ensure child process is terminated and reaped
        kill(state.pid, SIGKILL);
        waitpid(state.pid, NULL, WNOHANG);

        Logger::info("CGI Finished, processing response");
        processCgiResponse(client);
        
        client->setState(WRITING);
        client->getParser().reset();
        poller.modifyFd(client->getFd(), EPOLLOUT);
    }
}

void CgiHandler::processCgiResponse(ClientConnection* client)
{
    CgiState &state = client->getCgiState();
    HttpResponse response;
    response.setStatus(200, "OK");

    std::string &raw = state.responseBuffer;
    // Support both CRLF and LF for headers
    size_t headerEnd = raw.find("\r\n\r\n");
    size_t bodyStart = 0;

    if (headerEnd != std::string::npos)
    {
        bodyStart = headerEnd + 4;
    }
    else
    {
        headerEnd = raw.find("\n\n");
        if (headerEnd != std::string::npos)
        {
            bodyStart = headerEnd + 2;
        }
    }

    if (headerEnd != std::string::npos)
    {
        std::string headers = raw.substr(0, headerEnd);
        std::string body = raw.substr(bodyStart);

        std::istringstream stream(headers);
        std::string line;
        while (std::getline(stream, line))
        {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line.resize(line.size() - 1);

            size_t colon = line.find(':');
            if (colon != std::string::npos)
            {
                std::string key = line.substr(0, colon);
                std::string val = line.substr(colon + 1);
                size_t first = val.find_first_not_of(" \t");
                if (first != std::string::npos)
                    val = val.substr(first);

                if (key == "Status")
                {
                    size_t sp = val.find(' ');
                    if (sp != std::string::npos)
                        response.setStatus(std::atoi(val.c_str()), val.substr(sp + 1));
                    else
                        response.setStatus(std::atoi(val.c_str()), "OK");
                }
                else
                {
                    response.addHeader(key, val);
                }
            }
        }
        response.setBody(body);

        // Ensure Content-Length is set
        std::ostringstream ss;
        ss << body.size();
        response.addHeader("Content-Length", ss.str());
    }
    else
    {
        response.setBody(raw);
        std::ostringstream ss;
        ss << raw.size();
        response.addHeader("Content-Length", ss.str());
        response.addHeader("Content-Type", "text/plain"); // Default fallback
    }

    client->appendToWriteBuffer(response.build());
}
