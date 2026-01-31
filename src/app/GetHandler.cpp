#include "app/GetHandler.hpp"
#include "app/CgiExecutor.hpp"
#include <sstream>

HttpResponse GetHandler::handle(
    const HttpRequest &request,
    const LocationConfig &location)
{
    std::string uri = request.getUri();
    std::string rootDir = location.getRoot();
    
    // Default to a sane default if root is empty (should ideally be handled in config validation)
    if (rootDir.empty()) rootDir = DEFAULT_ROOT;

    Logger::debug("GetHandler processing: " + uri);

    // Security: Validate URI safety
    if (!isPathSafe(uri))
    {
        Logger::warn("Unsafe path detected: " + uri);
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }

    // Normalize URI (remove query strings, fragments)
    // NOTE: normalized URI is used for file lookups, but raw URI with query string is passed to CGI
    std::string safeUri = normalizeUri(uri);

    // Build file path
    std::string defaultIndex = DEFAULT_INDEX;
    const std::vector<std::string> &indices = location.getIndex();
    if (!indices.empty())
        defaultIndex = indices[0];

    std::string filePath = buildFilePath(safeUri, rootDir, defaultIndex);
    Logger::debug("Resolved file path: " + filePath);

    // Check for CGI
    if (isCgiRequest(filePath, location))
        return executeCgi(request, filePath, location);

    // Serve the file
    return serveFile(filePath);
}

bool GetHandler::isPathSafe(const std::string &uri)
{
    // Directory traversal attempts
    if (uri.find("..") != std::string::npos)
        return false;

    // Double slashes
    if (uri.find("//") != std::string::npos)
        return false;

    // Backslashes
    if (uri.find("\\") != std::string::npos)
        return false;

    // Null byte injection
    if (uri.find('\0') != std::string::npos)
        return false;

    return true;
}

std::string GetHandler::normalizeUri(const std::string &uri)
{
    // Find the first occurrence of either '?' (query) or '#' (fragment)
    size_t endPos = uri.find_first_of("?#");

    // If found, return everything before it
    if (endPos != std::string::npos)
        return uri.substr(0, endPos);

    // Otherwise, return the URI unchanged
    return uri;
}

HttpResponse GetHandler::serveFile(const std::string &filePath)
{
    // Check if file exists
    if (!FileHandler::fileExists(filePath))
    {
        Logger::debug("File not found: " + filePath);
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }

    // Check if it's a directory
    if (FileHandler::isDirectory(filePath))
    {
        Logger::debug("Path is a directory: " + filePath);
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }

    // Check if file is readable
    if (!FileHandler::isReadable(filePath))
    {
        Logger::warn("File not readable: " + filePath);
        return StatusCodes::createErrorResponse(HTTP_FORBIDDEN, "Forbidden");
    }

    // Read file content
    std::string content = FileHandler::readFile(filePath);
    if (content.empty())
    {
        size_t fileSize = FileHandler::getFileSize(filePath);
        if (fileSize > 0)
        {
            Logger::error("Failed to read file: " + filePath);
            return StatusCodes::createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error");
        }
        // File is actually empty (size 0), serve it
    }

    // Determine MIME type
    std::string mimeType = MimeTypes::getMimeType(filePath);

    // Build successful response
    HttpResponse response;
    response.setStatus(HTTP_OK, "OK")
        .addHeader("Content-Type", mimeType)
        .addHeader("Content-Length", toString(content.size()))
        .setBody(content);
    

    Logger::info("Served file: " + filePath + " (" + mimeType + ", " + toString(content.size()) + " bytes)");

    return response;
}


