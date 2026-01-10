#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include "utils/utils.hpp"
#include "utils/MimeTypes.hpp"
#include "utils/FileHandler.hpp"
#include "utils/ErrorPageGenerator.hpp"
#include <sstream>

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

void StatusCodes::configureErrorPage(int statusCode, const std::string &filePath)
{
    ErrorPageGenerator::getInstance().setErrorPage(statusCode, filePath);
}

void StatusCodes::clearErrorPage(int statusCode)
{
    ErrorPageGenerator::getInstance().clearErrorPage(statusCode);
}

void StatusCodes::clearAllErrorPages()
{
    ErrorPageGenerator::getInstance().clearAllErrorPages();
}

HttpResponse StatusCodes::createOkResponse(const std::string &filePath)
{
    if (!FileHandler::fileExists(filePath))
        return createNotFoundResponse();
    
    if (FileHandler::isDirectory(filePath))
        return createNotFoundResponse();
    
    if (!FileHandler::isReadable(filePath))
        return createErrorResponse(HTTP_FORBIDDEN, "Forbidden");

    std::string body = FileHandler::readFile(filePath);
    if (body.empty())
        return createServerErrorResponse();
    
    std::string contentType = MimeTypes::getMimeType(filePath);
         
    return buildResponse(HTTP_OK, "OK", body, contentType);
}

HttpResponse StatusCodes::createNotFoundResponse()
{
    return createErrorResponse(HTTP_NOT_FOUND, "Not Found");
}

HttpResponse StatusCodes::createServerErrorResponse()
{
    return createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error");
}


HttpResponse StatusCodes::createErrorResponse(int code, const std::string &reason)
{
    std::string body = ErrorPageGenerator::getInstance().getErrorPage(code, reason);
    return buildResponse(code, reason, body);
}