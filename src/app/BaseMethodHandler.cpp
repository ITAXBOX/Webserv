#include "app/BaseMethodHandler.hpp"

bool BaseMethodHandler::isCgiRequest(const std::string &path, const LocationConfig &config)
{
    std::string extension = "";
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
        extension = path.substr(dotPos);
    
    return !config.getCgiPath(extension).empty();
}

HttpResponse BaseMethodHandler::executeCgi(const std::string &path, const LocationConfig &config)
{
    Logger::info("CGI Request detected: " + path);
    
    // Determine interpreter
    std::string extension = "";
    size_t dotPos = path.rfind('.');
    if (dotPos != std::string::npos)
        extension = path.substr(dotPos);
        
    std::string interpreter = config.getCgiPath(extension);

    HttpResponse response;
    // Mark as CGI pending
    response.setCgi(true);
    response.setCgiInfo(path, interpreter);
    
    return response;
}
