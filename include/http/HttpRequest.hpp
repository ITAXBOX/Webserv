#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

// HTTP Methods
enum HttpMethod
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
	HTTP_PUT,
	HTTP_HEAD,
	HTTP_UNKNOWN
};

class HttpRequest
{
private:
	HttpMethod method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;

public:
	HttpRequest();
	~HttpRequest();

	HttpRequest &setMethod(HttpMethod method);
	HttpRequest &setUri(const std::string &uri);
	HttpRequest &setVersion(const std::string &version);
	HttpRequest &addHeader(const std::string &key, const std::string &value);
	HttpRequest &setBody(const std::string &body);

	HttpMethod getMethod() const;
	std::string getMethodString() const;
	std::string getUri() const;
	std::string getVersion() const;
	std::string getHeader(const std::string &key) const;
	std::string getBody() const;

	void clear();
};

#endif
