#include "utils/StatusCodes.hpp"

namespace
{
    HttpResponse buildResponse(int code, const std::string &reason, const std::string &body, const std::string &contentType = "text/html")
    {
        HttpResponse response;
        response.setStatus(code, reason)
            .addHeader("Content-Type", contentType)
            .addHeader("Content-Length", toString(body.size()))
            .setBody(body);
        return response;
    }
}

void StatusCodes::clearErrorPage(int statusCode)
{
    ErrorPageGenerator::getInstance().clearErrorPage(statusCode);
}

void StatusCodes::clearAllErrorPages()
{
    ErrorPageGenerator::getInstance().clearAllErrorPages();
}

HttpResponse StatusCodes::createErrorResponse(int code, const std::string &reason)
{
    std::string body = ErrorPageGenerator::getInstance().getErrorPage(code, reason);
    return buildResponse(code, reason, body);
}