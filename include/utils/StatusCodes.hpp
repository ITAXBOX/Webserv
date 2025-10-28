#ifndef STATUSCODES_HPP
#define STATUSCODES_HPP

#include "http/HttpResponse.hpp"

class StatusCodes
{
public:
    static HttpResponse createOkResponse(const std::string &body, const std::string &contentType = "text/html");
    static HttpResponse createNotFoundResponse();
    static HttpResponse createServerErrorResponse();
};

#endif