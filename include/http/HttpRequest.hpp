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
	std::string uri;							// The Uniform Resource Identifier - the path requested
	std::string version;						// 1.1
	std::map<std::string, std::string> headers; // metadata about the request - like the Host, Content-Type, etc.
	std::string body;
	std::map<std::string, std::string> cookies;

public:
	HttpRequest();
	~HttpRequest();

	HttpRequest &setMethod(HttpMethod method);
	HttpRequest &setUri(const std::string &uri);
	HttpRequest &setVersion(const std::string &version);
	HttpRequest &addHeader(const std::string &key, const std::string &value);
	HttpRequest &setBody(const std::string &body);
	void parseCookies();

	HttpMethod getMethod() const;
	std::string getMethodString() const;
	std::string getUri() const;
	std::string getHeader(const std::string &key) const;
	const std::map<std::string, std::string> &getHeaders() const;
	std::string getBody() const;
	std::string getCookie(const std::string &key) const;

	void clear();
};

#endif
