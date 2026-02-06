#include "http/HttpRequest.hpp"

HttpRequest::HttpRequest()
	: method(HTTP_UNKNOWN), uri(""), version("HTTP/1.1"), body("")
{
}

HttpRequest::~HttpRequest()
{
}

HttpRequest &HttpRequest::setMethod(HttpMethod m)
{
	method = m;
	return *this;
}

HttpRequest &HttpRequest::setUri(const std::string &u)
{
	uri = u;
	return *this;
}

HttpRequest &HttpRequest::setVersion(const std::string &v)
{
	version = v;
	return *this;
}

HttpRequest &HttpRequest::addHeader(const std::string &key, const std::string &value)
{
	headers[key] = value;
	return *this;
}

HttpRequest &HttpRequest::setBody(const std::string &b)
{
	body = b;
	return *this;
}

void HttpRequest::parseCookies()
{
	cookies.clear();
	std::string cookieHeader = getHeader("Cookie");
	if (cookieHeader.empty())
		return;

	size_t pos = 0;
	while (pos < cookieHeader.length())
	{
		size_t end = cookieHeader.find(';', pos);
		if (end == std::string::npos)
			end = cookieHeader.length();

		std::string pair = cookieHeader.substr(pos, end - pos);
		size_t eq = pair.find('=');
		if (eq != std::string::npos)
		{
			std::string key = pair.substr(0, eq);
			std::string val = pair.substr(eq + 1);

			// Trim whitespace
			size_t first = key.find_first_not_of(" ");
			if (first != std::string::npos)
				key = key.substr(first);

			cookies[key] = val;
		}

		pos = end + 1;
	}
}

std::string HttpRequest::getCookie(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = cookies.find(key);
	if (it != cookies.end())
		return it->second;
	return "";
}

HttpMethod HttpRequest::getMethod() const
{
	return method;
}

std::string HttpRequest::getMethodString() const
{
	switch (method)
	{
	case HTTP_GET:
		return "GET";
	case HTTP_POST:
		return "POST";
	case HTTP_DELETE:
		return "DELETE";
	case HTTP_PUT:
		return "PUT";
	case HTTP_HEAD:
		return "HEAD";
	default:
		return "UNKNOWN";
	}
}

std::string HttpRequest::getUri() const
{
	return uri;
}

std::string HttpRequest::getHeader(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it != headers.end())
		return it->second;
	return "";
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const
{
	return headers;
}

std::string HttpRequest::getBody() const
{
	return body;
}

void HttpRequest::clear()
{
	method = HTTP_UNKNOWN;
	uri.clear();
	version = "HTTP/1.1";
	headers.clear();
	body.clear();
}
