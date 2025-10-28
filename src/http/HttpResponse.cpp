#include "http/HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse()
    : statusCode(200), version("HTTP/1.1"), reasonPhrase("OK"), body("") {}

HttpResponse::~HttpResponse() {}

HttpResponse &HttpResponse::setStatus(int code, const std::string &reason)
{
    statusCode = code;
    reasonPhrase = reason;
    return *this;
}

HttpResponse &HttpResponse::addHeader(const std::string &key, const std::string &value)
{
    headers[key] = value;
    return *this;
}

HttpResponse &HttpResponse::setBody(const std::string &bodyContent)
{
    body = bodyContent;
    return *this;
}

std::string HttpResponse::build() const
{
    std::ostringstream response;

    // Status line
    response << version << " " << statusCode << " " << reasonPhrase << "\r\n";

    // Headers
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++)
        response << it->first << ": " << it->second << "\r\n";

    // Empty line between headers and body
    response << "\r\n";

    // Body
    response << body;

    return response.str();
}