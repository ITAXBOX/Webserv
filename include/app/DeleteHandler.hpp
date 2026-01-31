#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/FileHandler.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include "utils/utils.hpp"
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

class DeleteHandler : public IMethodHandler
{
public:
    DeleteHandler() {}
    ~DeleteHandler() {}

    HttpResponse handle(
        const HttpRequest &request,
        const LocationConfig &location);

    std::string getName() const { return "DeleteHandler"; }
};

#endif
