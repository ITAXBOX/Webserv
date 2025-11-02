#ifndef PUTHANDLER_HPP
#define PUTHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "utils/FileHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/utils.hpp"
#include "utils/defines.hpp"
#include <cerrno>
#include <cstring>
#include <fstream>

class PutHandler : public IMethodHandler
{
public:
    PutHandler() {}
    ~PutHandler() {}

    HttpResponse handle(
        const HttpRequest &request,
        const std::string &rootDir,
        const std::string &defaultIndex);

    std::string getName() const { return "PUT"; }
};

#endif
