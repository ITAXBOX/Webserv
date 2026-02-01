#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include "http/HttpRequest.hpp"
#include "core/CgiState.hpp"
#include <string>
#include <vector>
#include <map>

class CgiExecutor
{
public:
    CgiExecutor();
    ~CgiExecutor();

    // Starts CGI process and initializes CgiState pipes
    void start(const HttpRequest &request, const std::string &scriptPath, const std::string &interpreterPath, CgiState& state);

private:
    char **createEnvp(const HttpRequest &request, const std::string &scriptPath);
    char **createArgv(const std::string &scriptPath, const std::string &interpreterPath);
    void freeArray(char **array);
    
    std::string _programName; // argv[0]
};

#endif
