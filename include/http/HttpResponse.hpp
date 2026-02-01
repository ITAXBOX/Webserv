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
    
    bool _isCgi;
    std::string _cgiScriptPath;
    std::string _cgiInterpreterPath;

public:
    HttpResponse();
    ~HttpResponse();

    HttpResponse &setStatus(int code, const std::string &reason);
    HttpResponse &addHeader(const std::string &key, const std::string &value);
    HttpResponse &setBody(const std::string &body);
    std::string getHeader(const std::string &key) const;

    void setCgi(bool isCgi);
    bool isCgi() const;
    void setCgiInfo(const std::string &script, const std::string &interpreter);
    std::string getCgiScriptPath() const;
    std::string getCgiInterpreterPath() const;

    std::string build() const;
};

#endif