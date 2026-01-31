#include "app/BaseMethodHandler.hpp"
#include <cstdlib>

bool BaseMethodHandler::isCgiRequest(const std::string &path, const LocationConfig &config)
{
    std::string extension = "";
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
        extension = path.substr(dotPos);
    
    return !config.getCgiPath(extension).empty();
}

HttpResponse BaseMethodHandler::executeCgi(const HttpRequest &request, const std::string &path, const LocationConfig &config)
{
    Logger::info("Executing CGI script: " + path);
    
    CgiExecutor executor;
    
    // Determine interpreter
    std::string extension = "";
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
        extension = path.substr(dotPos);
        
    std::string interpreter = config.getCgiPath(extension);
        
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
