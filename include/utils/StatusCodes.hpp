#ifndef STATUSCODES_HPP
#define STATUSCODES_HPP

#include "http/HttpResponse.hpp"

class StatusCodes
{
public:
    // Configure custom error pages
    static void configureErrorPage(int statusCode, const std::string &filePath);
    static void clearErrorPage(int statusCode);
    static void clearAllErrorPages();
    
    // Create HTTP responses
    static HttpResponse createOkResponse(const std::string &filePath);
    static HttpResponse createNotFoundResponse();
    static HttpResponse createServerErrorResponse();
    static HttpResponse createErrorResponse(int code, const std::string &reason);
};

#endif