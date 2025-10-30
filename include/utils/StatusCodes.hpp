#ifndef STATUSCODES_HPP
#define STATUSCODES_HPP

#include "http/HttpResponse.hpp"

class StatusCodes
{
public:
    static HttpResponse createOkResponse(const std::string &filePath);
    static HttpResponse createNotFoundResponse();
    static HttpResponse createServerErrorResponse();
    static HttpResponse createErrorResponse(int code, const std::string &reason);
};

#endif