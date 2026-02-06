#ifndef PUTHANDLER_HPP
#define PUTHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "app/SessionHandler.hpp"
#include "app/HeadHandler.hpp"
#include "app/PutHandler.hpp"
#include "utils/FileHandler.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include <cstring>
#include <fstream>
#include <cerrno>

class PutHandler : public IMethodHandler
{
public:
    PutHandler() {}
    ~PutHandler() {}

    HttpResponse handle(
        const HttpRequest &request,
        const LocationConfig &location);

    std::string getName() const { return "PUT"; }
};

#endif
