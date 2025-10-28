#include "utils/StatusCodes.hpp"
#include <sstream>

HttpResponse StatusCodes::createOkResponse(const std::string &body, const std::string &contentType)
{
    HttpResponse response;
    response.setStatus(200, "OK")
            .addHeader("Content-Type", contentType)
            .addHeader("Content-Length", std::to_string(body.size()))
            .setBody(body);
    return response;
}

HttpResponse StatusCodes::createNotFoundResponse()
{
    std::string body = "<html><body><h1>404 Not Found</h1></body></html>";
    HttpResponse response;
    response.setStatus(404, "Not Found")
            .addHeader("Content-Type", "text/html")
            .addHeader("Content-Length", std::to_string(body.size()))
            .setBody(body);
    return response;
}

HttpResponse StatusCodes::createServerErrorResponse()
{
    std::string body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
    HttpResponse response;
    response.setStatus(500, "Internal Server Error")
            .addHeader("Content-Type", "text/html")
            .addHeader("Content-Length", std::to_string(body.size()))
            .setBody(body);
    return response;
}