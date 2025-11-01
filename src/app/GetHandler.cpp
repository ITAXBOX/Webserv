#include "app/GetHandler.hpp"
#include "app/FileHandler.hpp"
#include "app/app.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/utils.hpp"

GetHandler::GetHandler()
{
    Logger::debug("GetHandler created");
}

GetHandler::~GetHandler() {}

HttpResponse GetHandler::handle(
    const HttpRequest &request,
    const std::string &rootDir,
    const std::string &defaultIndex)
{
    std::string uri = request.getUri();

    Logger::debug("GetHandler processing: " + uri);

    // Security: Validate URI safety
    if (!isPathSafe(uri))
    {
        Logger::warn("Unsafe path detected: " + uri);
        return StatusCodes::createErrorResponse(404, "Not Found");
    }

    // Normalize URI (remove query strings, fragments)
    uri = normalizeUri(uri);

    // Build file path
    std::string filePath = buildFilePath(uri, rootDir, defaultIndex);
    Logger::debug("Resolved file path: " + filePath);

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

std::string GetHandler::buildFilePath(
    const std::string &uri,
    const std::string &rootDir,
    const std::string &defaultIndex)
{
    std::string filePath = rootDir;

    // Handle root URI or empty URI
    if (uri == "/" || uri.empty())
        filePath += "/" + defaultIndex;
    else
    {
        // Append URI to root directory
        filePath += uri;

        // If URI points to a directory, append default index
        if (FileHandler::isDirectory(filePath))
        {
            // Ensure trailing slash
            if (filePath[filePath.length() - 1] != '/')
                filePath += "/";
            filePath += defaultIndex;
        }
    }

    return filePath;
}

HttpResponse GetHandler::serveFile(const std::string &filePath)
{
    // Check if file exists
    if (!FileHandler::fileExists(filePath))
    {
        Logger::debug("File not found: " + filePath);
        return StatusCodes::createErrorResponse(404, "Not Found");
    }

    // Check if it's a directory
    if (FileHandler::isDirectory(filePath))
    {
        Logger::debug("Path is a directory: " + filePath);
        return StatusCodes::createErrorResponse(404, "Not Found");
    }

    // Check if file is readable
    if (!FileHandler::isReadable(filePath))
    {
        Logger::warn("File not readable: " + filePath);
        return StatusCodes::createErrorResponse(403, "Forbidden");
    }

    // Read file content
    std::string content = FileHandler::readFile(filePath);
    if (content.empty())
    {
        size_t fileSize = FileHandler::getFileSize(filePath);
        if (fileSize > 0)
        {
            Logger::error("Failed to read file: " + filePath);
            return StatusCodes::createErrorResponse(500, "Internal Server Error");
        }
        // File is actually empty (size 0), serve it
    }

    // Determine MIME type
    std::string mimeType = MimeTypes::getMimeType(filePath);

    // Build successful response
    HttpResponse response;
    response.setStatus(200, "OK")
        .addHeader("Content-Type", mimeType)
        .addHeader("Content-Length", toString(content.size()))
        .setBody(content);
    

    Logger::info("Served file: " + filePath + " (" + mimeType + ", " + toString(content.size()) + " bytes)");

    return response;
}