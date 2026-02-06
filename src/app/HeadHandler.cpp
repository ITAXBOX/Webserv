#include "app/HeadHandler.hpp"

HttpResponse HeadHandler::handle(
    const HttpRequest &request,
    const LocationConfig &location)
{
    std::string rootDir = location.getRoot();
    if (rootDir.empty())
        rootDir = DEFAULT_ROOT;
    std::string defaultIndex = DEFAULT_INDEX;
    if (!location.getIndex().empty())
        defaultIndex = location.getIndex()[0];

    std::string path = buildFilePath(request.getUri(), rootDir, defaultIndex);
    Logger::info("HEAD request for: " + path);

    // Check if file exists
    if (!FileHandler::fileExists(path))
    {
        Logger::warn("File not found: " + path);
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }

    // Check readability
    if (!FileHandler::isReadable(path))
    {
        Logger::warn("File not readable: " + path);
        return StatusCodes::createErrorResponse(HTTP_FORBIDDEN, "Forbidden");
    }

    // Build headers
    HttpResponse res;
    res.setStatus(HTTP_OK, "OK");

    std::string mime = MimeTypes::getMimeType(path);
    size_t fileSize = FileHandler::getFileSize(path);

    res.addHeader("Content-Type", mime);
    res.addHeader("Content-Length", toString(fileSize));

    // No body for HEAD
    return res;
}
