#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include "http/HttpRequest.hpp"
#include <string>
#include <vector>
#include <map>

class CgiExecutor
{
public:
    CgiExecutor();
    ~CgiExecutor();

    std::string execute(const HttpRequest &request, const std::string &scriptPath, const std::string &interpreterPath);

private:
    char **createEnvp(const HttpRequest &request, const std::string &scriptPath);
    char **createArgv(const std::string &scriptPath, const std::string &interpreterPath);
    void freeArray(char **array);
    
    std::string _programName; // argv[0]
};

#endif
