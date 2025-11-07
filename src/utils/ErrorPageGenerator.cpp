#include "utils/ErrorPageGenerator.hpp"
#include "utils/FileHandler.hpp"
#include <sstream>

ErrorPageHandler& ErrorPageHandler::getInstance()
{
    static ErrorPageHandler instance;
    return instance;
}

void ErrorPageHandler::setErrorPage(int statusCode, const std::string &filePath)
{
    if (FileHandler::fileExists(filePath) && !FileHandler::isDirectory(filePath))
        errorPages_[statusCode] = filePath;
}

void ErrorPageHandler::clearErrorPage(int statusCode)
{
    errorPages_.erase(statusCode);
}

void ErrorPageHandler::clearAllErrorPages()
{
    errorPages_.clear();
}

std::string ErrorPageHandler::getErrorPage(int statusCode, const std::string &reason) const
{
    // Check if custom error page exists
    std::map<int, std::string>::const_iterator it = errorPages_.find(statusCode);
    if (it != errorPages_.end())
    {
        std::string content = FileHandler::readFile(it->second);
        if (!content.empty())
        {
            // Replace placeholders in custom error page
            std::string result = content;
            
            // Replace {STATUS_CODE} placeholder
            std::ostringstream codeStr;
            codeStr << statusCode;
            size_t pos = 0;
            while ((pos = result.find("{STATUS_CODE}", pos)) != std::string::npos)
            {
                result.replace(pos, 13, codeStr.str());
                pos += codeStr.str().length();
            }
            
            // Replace {REASON} placeholder
            pos = 0;
            while ((pos = result.find("{REASON}", pos)) != std::string::npos)
            {
                result.replace(pos, 8, reason);
                pos += reason.length();
            }
            
            return result;
        }
    }
    
    // Fall back to default error page
    return generateDefaultPage(statusCode, reason);
}

bool ErrorPageHandler::hasCustomPage(int statusCode) const
{
    return errorPages_.find(statusCode) != errorPages_.end();
}

std::string ErrorPageHandler::generateDefaultPage(int statusCode, const std::string &reason) const
{
    std::ostringstream html;
    html << "<html><body><h1>" << statusCode << " " << reason << "</h1></body></html>";
    return html.str();
}
