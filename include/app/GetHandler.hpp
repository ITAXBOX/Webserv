#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "app/BaseMethodHandler.hpp"
#include "utils/FileHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/utils.hpp"
#include "utils/defines.hpp"
#include "utils/MimeTypes.hpp"

// GetHandler - Strategy for handling HTTP GET requests
// Serves static files (HTML, CSS, images, etc.)
// Implements security checks and MIME type detection

class GetHandler : public BaseMethodHandler
{
public:
	GetHandler() {}
	~GetHandler() {}

	// IMethodHandler interface implementation
	HttpResponse handle(
		const HttpRequest &request,
		const LocationConfig &location);

	std::string getName() const { return "GET"; }

private:
	// Path handling and security
	bool isPathSafe(const std::string &uri);
	std::string normalizeUri(const std::string &uri);

	// File serving
	HttpResponse serveFile(const std::string &filePath, bool autoindex);
	HttpResponse generateAutoIndex(const std::string &dirPath);

};

#endif