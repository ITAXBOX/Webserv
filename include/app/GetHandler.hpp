#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include <string>

// GetHandler - Strategy for handling HTTP GET requests
// Serves static files (HTML, CSS, images, etc.)
// Implements security checks and MIME type detection

class GetHandler : public IMethodHandler
{
public:
	GetHandler();
	~GetHandler();

	// IMethodHandler interface implementation
	HttpResponse handle(
		const HttpRequest &request,
		const std::string &rootDir,
		const std::string &defaultIndex);

	std::string getName() const { return "GET"; }

private:
	// Path handling and security
	bool isPathSafe(const std::string &uri);
	std::string normalizeUri(const std::string &uri);

	// File serving
	HttpResponse serveFile(const std::string &filePath);
};

#endif