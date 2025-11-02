#include "app/DeleteHandler.hpp"
#include "app/FileHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/utils.hpp"
#include "utils/defines.hpp"
#include <cstring>
#include <cerrno>

HttpResponse DeleteHandler::handle(const HttpRequest &request,
                                   const std::string &rootDir,
                                   const std::string &defaultIndex)
{
	HttpResponse response;

	std::string path = buildFilePath(request.getUri(), rootDir, defaultIndex);
	Logger::info("DELETE request for: " + path);

	// Check if file exists
	if (!FileHandler::fileExists(path))
	{
		Logger::warn("File not found: " + path);
		return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "File not found");
	}

	// Ensure it’s not a directory
	if (FileHandler::isDirectory(path))
	{
		Logger::warn("Attempt to delete directory: " + path);
		return StatusCodes::createErrorResponse(HTTP_FORBIDDEN, "Cannot delete directories");
	}

	// Attempt to delete file
	if (unlink(path.c_str()) != 0)
	{
		Logger::error("Failed to delete file (" + path + "): " + std::string(strerror(errno)));
		return StatusCodes::createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, strerror(errno));
	}

	// Return success response
	Logger::info("File deleted successfully: " + path);

	response.setStatus(HTTP_NO_CONTENT, "No Content");
	response.addHeader("Content-Length", "0");
	response.addHeader("Connection", "close");

	return response;
}
