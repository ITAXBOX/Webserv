#include "app/DeleteHandler.hpp"

HttpResponse DeleteHandler::handle(
    const HttpRequest &request,
    const std::string &rootDir,
    const std::string &defaultIndex)
{
    (void)defaultIndex;

    std::string path = buildFilePath(request.getUri(), rootDir, "");
    Logger::info("DELETE request for: " + path);

    // Check if file exists
    if (!FileHandler::fileExists(path))
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "File not found");

    // Delete the file
    if (remove(path.c_str()) != 0)
    {
        Logger::error("Failed to delete file (" + path + "): " + std::string(strerror(errno)));
        return StatusCodes::createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, strerror(errno));
    }

    Logger::info("File deleted successfully: " + path);

    HttpResponse res;
    res.setStatus(HTTP_OK, "File deleted")
       .addHeader("Content-Type", "text/plain")
       .setBody("File successfully deleted");

    return res;
}
