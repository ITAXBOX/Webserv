#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include "utils/utils.hpp"
#include "app/app.hpp"
#include "utils/FileHandler.hpp"
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

HttpResponse StatusCodes::createOkResponse(const std::string &filePath)
{
    if (!FileHandler::fileExists(filePath))
        return createNotFoundResponse();
    
    if (FileHandler::isDirectory(filePath))
        return createNotFoundResponse();
    
    if (!FileHandler::isReadable(filePath))
    {
        std::string body = "<html><body><h1>403 Forbidden</h1><p>Permission denied</p></body></html>";
        return buildResponse(HTTP_FORBIDDEN, "Forbidden", body);
    }

    std::string body = FileHandler::readFile(filePath);
    if (body.empty())
        return createServerErrorResponse();
    
    std::string contentType = MimeTypes::getMimeType(filePath);
         
    return buildResponse(HTTP_OK, "OK", body, contentType);
}

HttpResponse StatusCodes::createNotFoundResponse()
{
    std::string body = "<html><body><h1>404 Not Found</h1></body></html>";
    return buildResponse(HTTP_NOT_FOUND, "Not Found", body);
}

HttpResponse StatusCodes::createServerErrorResponse()
{
    std::string body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
    return buildResponse(HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error", body);
}

HttpResponse StatusCodes::createErrorResponse(int code, const std::string &reason)
{
    std::ostringstream body;
    body << "<html><body><h1>" << code << " " << reason << "</h1></body></html>";
    return buildResponse(code, reason, body.str());
}
