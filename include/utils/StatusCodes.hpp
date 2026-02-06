#ifndef STATUSCODES_HPP
#define STATUSCODES_HPP

#include "http/HttpResponse.hpp"
#include "utils/ErrorPageGenerator.hpp"
#include "utils/FileHandler.hpp"
#include "utils/MimeTypes.hpp"
#include "utils/defines.hpp"
#include "utils/utils.hpp"
#include <sstream>

class StatusCodes
{
public:
    // Configure custom error pages
    static void clearErrorPage(int statusCode);
    static void clearAllErrorPages();
    
    static HttpResponse createNotFoundResponse();
    static HttpResponse createServerErrorResponse();
    static HttpResponse createErrorResponse(int code, const std::string &reason);
};

#endif