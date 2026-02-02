#include "app/GetHandler.hpp"
#include "app/CgiExecutor.hpp"
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>

HttpResponse GetHandler::handle(
    const HttpRequest &request,
    const LocationConfig &location)
{
    // Handle Redirection
    if (location.hasRedirect())
    {
        Logger::info("Redirecting request");
        HttpResponse response;
        int code = location.getRedirectCode();
        std::string reason = "Redirect";
        if (code == 301) reason = "Moved Permanently";
        else if (code == 302) reason = "Found";
        else if (code == 307) reason = "Temporary Redirect";
        else if (code == 308) reason = "Permanent Redirect";
        
        response.setStatus(code, reason);
        response.addHeader("Location", location.getRedirect());
        return response;
    }

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
    // We initially build the path without the index file
    std::string filePath = buildFilePath(safeUri, rootDir, "");
    
    // If it's a directory, check if we should serve the index file
    if (FileHandler::isDirectory(filePath))
    {
        std::string defaultIndex = DEFAULT_INDEX;
        const std::vector<std::string> &indices = location.getIndex();
        if (!indices.empty())
            defaultIndex = indices[0];
            
        std::string indexPath = filePath;
        if (!indexPath.empty() && indexPath[indexPath.size()-1] != '/') 
            indexPath += "/";
        indexPath += defaultIndex;
        
        // If the index file exists, we serve that instead of the directory
        if (FileHandler::fileExists(indexPath))
        {
            filePath = indexPath;
        }
        // Otherwise, we keep filePath as the directory path, which will trigger autoindex (if enabled)
        // or a 404/403 in serveFile
    }

    Logger::debug("Resolved file path: " + filePath);

    // Check for CGI
    if (isCgiRequest(filePath, location))
        return executeCgi(filePath, location);

    // Serve the file
    return serveFile(filePath, location.getAutoindex());
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

HttpResponse GetHandler::serveFile(const std::string &filePath, bool autoindex)
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
        if (autoindex)
             return generateAutoIndex(filePath);
        
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

HttpResponse GetHandler::generateAutoIndex(const std::string &dirPath)
{
    DIR *dir = opendir(dirPath.c_str());
    if (dir == NULL)
    {
        Logger::error("Failed to open directory for autoindex: " + dirPath);
        return StatusCodes::createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error");
    }

    std::ostringstream html;
    html << "<html><head><title>Index of " << dirPath << "</title></head><body>";
    html << "<h1>Index of " << dirPath << "</h1><hr><pre>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == ".") continue;
        
        // Add trailing slash for directories
        std::string fullPath = dirPath;
        if (fullPath[fullPath.length()-1] != '/') fullPath += "/";
        fullPath += name;
        
        bool isDir = FileHandler::isDirectory(fullPath);
        if (isDir) name += "/";
        
        // Date and Size
        struct stat st;
        std::string dateStr = "                   "; // 19 spaces
        std::string sizeStr = "        -";
        
        if (stat(fullPath.c_str(), &st) == 0)
        {
            char buf[100];
            struct tm *tm = std::localtime(&st.st_mtime);
            std::strftime(buf, sizeof(buf), "%d-%b-%Y %H:%M", tm);
            dateStr = std::string(buf);
            if (dateStr.length() < 19) dateStr.resize(19, ' ');
            
            if (!isDir)
                sizeStr = toString(static_cast<unsigned long>(st.st_size));
        }

        // Output link (adjust spacing as needed)
        // <a href="name">name</a>      date       size
        html << "<a href=\"" << name << "\">" << name << "</a>";
        
        // Padding
        int pad = 50 - name.length();
        if (pad < 0) pad = 0;
        html << std::string(pad, ' ');
        
        html << dateStr << "       " << sizeStr << "\n";
    }

    html << "</pre><hr></body></html>";
    closedir(dir);

    HttpResponse response;
    response.setStatus(HTTP_OK, "OK");
    response.setBody(html.str());
    response.addHeader("Content-Type", "text/html");
    response.addHeader("Content-Length", toString(html.str().length()));
    
    return response;
}


