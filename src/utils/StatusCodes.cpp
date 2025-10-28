#include "utils/StatusCodes.hpp"
#include <sstream>

namespace
{
    std::string toString(size_t n)
    {
        std::ostringstream os;
        os << n;
        return os.str();
    }

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

HttpResponse StatusCodes::createOkResponse(const std::string &body, const std::string &contentType)
{
    return buildResponse(200, "OK", body, contentType);
}

HttpResponse StatusCodes::createNotFoundResponse()
{
    std::string body = "<html><body><h1>404 Not Found</h1></body></html>";
    return buildResponse(404, "Not Found", body);
}

HttpResponse StatusCodes::createServerErrorResponse()
{
    std::string body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
    return buildResponse(500, "Internal Server Error", body);
}