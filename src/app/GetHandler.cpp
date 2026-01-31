#include "app/GetHandler.hpp"
#include "app/CgiExecutor.hpp"
#include <sstream>

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
        return StatusCodes::createErrorResponse(HTTP_NOT_FOUND, "Not Found");
    }

    // Normalize URI (remove query strings, fragments)
    // NOTE: normalized URI is used for file lookups, but raw URI with query string is passed to CGI
    std::string safeUri = normalizeUri(uri);

    // Build file path
    std::string filePath = buildFilePath(safeUri, rootDir, defaultIndex);
    Logger::debug("Resolved file path: " + filePath);

    // Check for CGI
    if (isCgiRequest(filePath))
        return executeCgi(request, filePath);

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

bool GetHandler::isCgiRequest(const std::string &path)
{
    std::string extension = "";
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
        extension = path.substr(dotPos);
    
    // Check for common CGI extensions
    // Note: In a full implementation, this should check the Location configuration
    if (extension == ".php" || extension == ".py" || extension == ".cgi" || extension == ".pl")
        return true;
        
    return false;
}

HttpResponse GetHandler::executeCgi(const HttpRequest &request, const std::string &path)
{
    Logger::info("Executing CGI script: " + path);
    
    CgiExecutor executor;
    
    // Determine interpreter based on extension (Fallback mechanism)
    std::string interpreter = ""; 
    if (path.rfind(".py") != std::string::npos)
        interpreter = "python3";
    else if (path.rfind(".php") != std::string::npos)
        interpreter = "php-cgi";
        
    // Execute the script
    std::string cgiOutput = executor.execute(request, path, interpreter);
    
    // Check for execution failure (empty output isn't necessarily a failure, but often is for CGI)
    if (cgiOutput.empty())
    {
        Logger::error("CGI execution returned empty output or failed");
        return StatusCodes::createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, "CGI Error");
    }

    HttpResponse response;
    // Default to OK, CGI can override
    response.setStatus(HTTP_OK, "OK"); 

    // Parse CGI Output
    // CGI scripts output headers (like Content-Type) followed by a blank line, then the body
    size_t headerEnd = cgiOutput.find("\r\n\r\n");
    if (headerEnd != std::string::npos)
    {
        std::string headersPart = cgiOutput.substr(0, headerEnd);
        std::string bodyPart = cgiOutput.substr(headerEnd + 4);
        
        std::istringstream headerStream(headersPart);
        std::string line;
        while (std::getline(headerStream, line))
        {
            // Handle CRLF in line reading
            if (!line.empty() && line[line.length()-1] == '\r')
                line.erase(line.length()-1);
                
            if (line.empty()) continue;
            
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                
                // Trim leading spaces from value
                while (!value.empty() && value[0] == ' ') 
                    value.erase(0, 1);
                
                if (key == "Status")
                {
                    // Parse custom Status header: "Status: 404 Not Found"
                    size_t spacePos = value.find(' ');
                    if (spacePos != std::string::npos)
                    {
                         int code = std::atoi(value.substr(0, spacePos).c_str());
                         std::string msg = value.substr(spacePos + 1);
                         response.setStatus(code, msg);
                    }
                    else
                        response.setStatus(std::atoi(value.c_str()), "Unknown");
                }
                else
                    response.addHeader(key, value);
            }
        }
        response.setBody(bodyPart);
    }
    else
    {
        // Fallback if no headers found (relaxed mode)
        Logger::warn("CGI output invalid (no headers/body separator), serving as plain text");
        response.setBody(cgiOutput);
        response.addHeader("Content-Type", "text/plain");
    }

    return response;
}