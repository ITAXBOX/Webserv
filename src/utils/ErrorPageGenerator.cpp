#include "utils/ErrorPageGenerator.hpp"

ErrorPageGenerator &ErrorPageGenerator::getInstance()
{
    static ErrorPageGenerator instance;
    return instance;
}

void ErrorPageGenerator::setErrorPage(int statusCode, const std::string &filePath)
{
    // CACHE IMPROVEMENT: Read file immediately into memory
    if (FileHandler::fileExists(filePath) && !FileHandler::isDirectory(filePath))
    {
        std::string content = FileHandler::readFile(filePath);
        if (!content.empty())
            errorPages_[statusCode] = content; // Store content, not path
    }
}

void ErrorPageGenerator::clearErrorPage(int statusCode)
{
    errorPages_.erase(statusCode);
}

void ErrorPageGenerator::clearAllErrorPages()
{
    errorPages_.clear();
}

std::string ErrorPageGenerator::getErrorPage(int statusCode, const std::string &reason) const
{
    std::string content;

    // Check if custom error page exists in cache
    std::map<int, std::string>::const_iterator it = errorPages_.find(statusCode);
    if (it != errorPages_.end())
        content = it->second;
    else
        content = generateDefaultPage();

    // Replace placeholders: {STATUS_CODE} and {REASON}
    // We do this for both custom and default pages to ensure consistency
    std::ostringstream codeStr;
    codeStr << statusCode;
    std::string codeS = codeStr.str();

    size_t pos = 0;
    while ((pos = content.find("{STATUS_CODE}", pos)) != std::string::npos)
    {
        content.replace(pos, 13, codeS);
        pos += codeS.length();
    }

    pos = 0;
    while ((pos = content.find("{REASON}", pos)) != std::string::npos)
    {
        content.replace(pos, 8, reason);
        pos += reason.length();
    }

    return content;
}

bool ErrorPageGenerator::hasCustomPage(int statusCode) const
{
    return errorPages_.find(statusCode) != errorPages_.end();
}

std::string ErrorPageGenerator::generateDefaultPage() const
{
    return "<!DOCTYPE html>"
           "<html lang=\"en\">"
           "<head>"
           "<meta charset=\"UTF-8\">"
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
           "<title>{STATUS_CODE} - {REASON}</title>"
           "<style>"
           "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f4f4f9; color: #333; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }"
           ".container { text-align: center; padding: 40px; background: white; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); max-width: 500px; width: 100%; }"
           "h1 { font-size: 6rem; margin: 0; color: #e74c3c; line-height: 1; }"
           "h2 { font-size: 1.5rem; margin: 10px 0 20px; color: #555; }"
           "p { color: #888; margin-bottom: 30px; }"
           ".btn { display: inline-block; padding: 10px 20px; background-color: #3498db; color: white; text-decoration: none; border-radius: 5px; transition: background 0.3s; }"
           ".btn:hover { background-color: #2980b9; }"
           ".footer { margin-top: 20px; font-size: 0.8rem; color: #aaa; }"
           "</style>"
           "</head>"
           "<body>"
           "<div class=\"container\">"
           "<h1>{STATUS_CODE}</h1>"
           "<h2>{REASON}</h2>"
           "<p>The page you are looking for might have been removed, had its name changed, or is temporarily unavailable.</p>"
           "<a href=\"/\" class=\"btn\">Go to Homepage</a>"
           "<div class=\"footer\">Webserv Server</div>"
           "</div>"
           "</body>"
           "</html>";
}
