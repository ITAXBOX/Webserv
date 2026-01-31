#ifndef HEADHANDLER_HPP
#define HEADHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "utils/FileHandler.hpp"
#include "utils/MimeTypes.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/utils.hpp"
#include "utils/defines.hpp"

class HeadHandler : public IMethodHandler
{
public:
    HeadHandler() {}
    ~HeadHandler() {}

    HttpResponse handle(
        const HttpRequest &request,
        const LocationConfig &location);

    std::string getName() const { return "HEAD"; }
};

#endif
