#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include "core/CgiState.hpp"
#include "http/HttpRequest.hpp"
#include "utils/defines.hpp"
#include "utils/Logger.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

class CgiExecutor
{
public:
    CgiExecutor();
    ~CgiExecutor();

    // Starts CGI process and initializes CgiState pipes
    void start(const HttpRequest &request, const std::string &scriptPath, const std::string &interpreterPath, CgiState &state);

private:
    char **createEnvp(const HttpRequest &request, const std::string &scriptPath);
    char **createArgv(const std::string &scriptPath, const std::string &interpreterPath);

    std::string _programName; // argv[0]
};

#endif
