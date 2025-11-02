#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include <string>
#include <unistd.h>

class DeleteHandler : public IMethodHandler
{
public:
    DeleteHandler() {}
    ~DeleteHandler() {}

    HttpResponse handle(
        const HttpRequest &request,
        const std::string &rootDir,
        const std::string &defaultIndex);

    std::string getName() const { return "DeleteHandler"; }
};

#endif
