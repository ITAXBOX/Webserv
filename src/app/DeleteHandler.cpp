#include "app/DeleteHandler.hpp"

HttpResponse DeleteHandler::handle(const HttpRequest &request,
                                   const std::string &rootDir,
                                   const std::string &defaultIndex)
{
    std::string path = buildFilePath(request.getUri(), rootDir, defaultIndex);
    Logger::info("DELETE " + path);

    // Check if file exists
    if (!FileHandler::fileExists(path))
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "File not found");

    // Check if it's a directory
    if (FileHandler::isDirectory(path))
        return StatusCodes::createErrorResponse(HTTP_FORBIDDEN, "Cannot delete directory");

    // Truncate (simulate delete)
    int fd = open(path.c_str(), O_WRONLY | O_TRUNC);
    if (fd < 0)
    {
        Logger::error("Failed to open file for truncation: " + std::string(strerror(errno)));
        return StatusCodes::createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, strerror(errno));
    }
    close(fd);

    Logger::info("File truncated (simulated delete): " + path);

    HttpResponse response;
    response.setStatus(HTTP_OK, "File deleted")
    		.addHeader("Content-Type", "text/plain")
    		.setBody("File successfully deleted (truncated)");
    return response;
}
