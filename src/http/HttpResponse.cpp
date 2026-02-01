#include "http/HttpResponse.hpp"
#include "utils/defines.hpp"
#include <sstream>

HttpResponse::HttpResponse()
    : statusCode(HTTP_OK), version("HTTP/1.1"), reasonPhrase("OK"), body(""), 
      _isCgi(false), _cgiScriptPath(""), _cgiInterpreterPath("") {}

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

std::string HttpResponse::getHeader(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end())
        return it->second;
    return "";
}

void HttpResponse::setCgi(bool isCgi) { _isCgi = isCgi; }
bool HttpResponse::isCgi() const { return _isCgi; }
void HttpResponse::setCgiInfo(const std::string &script, const std::string &interpreter) {
    _cgiScriptPath = script;
    _cgiInterpreterPath = interpreter;
}
std::string HttpResponse::getCgiScriptPath() const { return _cgiScriptPath; }
std::string HttpResponse::getCgiInterpreterPath() const { return _cgiInterpreterPath; }

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
