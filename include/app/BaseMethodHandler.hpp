#ifndef BASEMETHODHANDLER_HPP
#define BASEMETHODHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "app/CgiExecutor.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include "utils/Logger.hpp"
#include <cstdlib>
#include <sstream>
#include <string>

class BaseMethodHandler : public IMethodHandler
{
public:
    virtual ~BaseMethodHandler() {}

    // Abstract methods from IMethodHandler still need to be implemented by children
    // virtual HttpResponse handle(...) = 0;
    // virtual std::string getName() const = 0;

protected:
    // Shared CGI logic
    bool isCgiRequest(const std::string &path, const LocationConfig &config);
    HttpResponse executeCgi(const std::string &path, const LocationConfig &config);
};

#endif
