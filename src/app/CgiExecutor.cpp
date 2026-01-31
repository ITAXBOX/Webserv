#include "app/CgiExecutor.hpp"
#include "utils/Logger.hpp"
#include "utils/defines.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>

CgiExecutor::CgiExecutor() {}

CgiExecutor::~CgiExecutor() {}

std::string CgiExecutor::execute(const HttpRequest &request, const std::string &scriptPath, const std::string &interpreterPath)
{
    int pipeIn[2];  // Parent writes to child stdin
    int pipeOut[2]; // Child writes to parent stdout

    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
    {
        Logger::error("Failed to create pipes for CGI");
        return "";
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        Logger::error("Failed to fork for CGI");
        return "";
    }

    if (pid == 0)
    {
        // Close unused ends
        close(pipeIn[1]);
        close(pipeOut[0]);

        // Redirect stdin
        dup2(pipeIn[0], STDIN_FILENO);
        close(pipeIn[0]);

        // Redirect stdout
        dup2(pipeOut[1], STDOUT_FILENO);
        close(pipeOut[1]);

        // Create environment and args
        char **envp = createEnvp(request, scriptPath);
        char **argv = createArgv(scriptPath, interpreterPath);

        // Execute
        execve(argv[0], argv, envp);
        
        // If execve returns, it failed
        Logger::error("execve failed for: " + scriptPath);
        exit(1);
    }
    else
    {
        // Close unused ends
        close(pipeIn[0]);
        close(pipeOut[1]);

        // Write request body to CGI (if POST)
        std::string body = request.getBody();
        if (!body.empty())
            write(pipeIn[1], body.c_str(), body.size());

        close(pipeIn[1]); // Close to signal EOF to child

        // Read CGI output
        std::string output;
        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0)
            output.append(buffer, bytesRead);

        close(pipeOut[0]);

        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            Logger::error("CGI script exited with error code");

        return output;
    }
}

char **CgiExecutor::createArgv(const std::string &scriptPath, const std::string &interpreterPath)
{
    char **argv = new char*[3];
    
    // If interpreter is provided (e.g. /usr/bin/python3), use it locally
    // argv[0] = interpreter, argv[1] = script, argv[2] = NULL
    if (!interpreterPath.empty())
    {
        argv[0] = strdup(interpreterPath.c_str());
        argv[1] = strdup(scriptPath.c_str());
        argv[2] = NULL;
    }
    else
    {
        // Execute directly (requires executable permissions and shebang)
        argv[0] = strdup(scriptPath.c_str());
        argv[1] = NULL;
        argv[2] = NULL;
    }
    
    return argv;
}

char **CgiExecutor::createEnvp(const HttpRequest &request, const std::string &scriptPath)
{
    std::map<std::string, std::string> env;
    
    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_SOFTWARE"] = "Webserv/1.0";
    
    env["REQUEST_METHOD"] = request.getMethodString();
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SCRIPT_NAME"] = scriptPath; // Should be relative to root ideally
    env["QUERY_STRING"] = ""; // Extract from URI if needed
    
    // Extract Query String from URI
    std::string uri = request.getUri();
    size_t qPos = uri.find('?');
    if (qPos != std::string::npos)
        env["QUERY_STRING"] = uri.substr(qPos + 1);
    
    // Headers to Env
    std::string contentLength = request.getHeader("Content-Length");
    if (!contentLength.empty())
        env["CONTENT_LENGTH"] = contentLength;
        
    std::string contentType = request.getHeader("Content-Type");
    if (!contentType.empty())
        env["CONTENT_TYPE"] = contentType;
        
    // Convert to char**
    char **envp = new char*[env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
    {
        std::string s = it->first + "=" + it->second;
        envp[i++] = strdup(s.c_str());
    }
    envp[i] = NULL;
    
    return envp;
}

void CgiExecutor::freeArray(char **array)
{
    if (!array) return;
    for (int i = 0; array[i]; ++i)
        free(array[i]);
    delete[] array;
}
