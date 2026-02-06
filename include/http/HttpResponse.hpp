#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "utils/defines.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <map>

class HttpResponse
{
private:
    int statusCode;
    std::string version;
    std::string reasonPhrase;
    std::map<std::string, std::string> headers;
    std::string body;

    bool _isCgi;
    std::string _cgiScriptPath;
    std::string _cgiInterpreterPath;
    std::vector<std::string> cookies;

public:
    HttpResponse();
    ~HttpResponse();

    HttpResponse &setStatus(int code, const std::string &reason);
    HttpResponse &addHeader(const std::string &key, const std::string &value);
    HttpResponse &setBody(const std::string &body);
    HttpResponse &addCookie(const std::string &key, const std::string &value, int maxAge = 0);
    std::string getHeader(const std::string &key) const;

    void setCgi(bool isCgi);
    bool isCgi() const;
    void setCgiInfo(const std::string &script, const std::string &interpreter);
    std::string getCgiScriptPath() const;
    std::string getCgiInterpreterPath() const;

    std::string build() const;
};

#endif