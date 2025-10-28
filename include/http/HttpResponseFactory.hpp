#ifndef HTTPRESPONSEFACTORY_HPP
#define HTTPRESPONSEFACTORY_HPP

#include "http/HttpResponse.hpp"

class HttpResponseFactory
{
public:
    static HttpResponse createOkResponse(const std::string &body, const std::string &contentType = "text/html");
    static HttpResponse createNotFoundResponse();
    static HttpResponse createServerErrorResponse();
};

#endif