#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HttpResponse
{
private:
    int statusCode;
    std::string version;
    std::string reasonPhrase;
    std::map<std::string, std::string> headers;
    std::string body;

public:
    HttpResponse();
    ~HttpResponse();

    HttpResponse &setStatus(int code, const std::string &reason);
    HttpResponse &addHeader(const std::string &key, const std::string &value);
    HttpResponse &setBody(const std::string &body);

    std::string build() const;
};

#endif