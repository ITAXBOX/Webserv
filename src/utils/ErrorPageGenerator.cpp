#include "utils/ErrorPageGenerator.hpp"

std::map<int, std::string> getErrorMessages()
{
    std::map<int, std::string> messages;
    messages[400] = "Bad Request";
    messages[403] = "Forbidden";
    messages[404] = "Not Found";
    messages[405] = "Method Not Allowed";
    messages[500] = "Internal Server Error";
    return messages;
}

// Map of error codes to their default messages

// Generate default HTML error page
std::string generateDefaultHTML(int code, const std::string &message)
{
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "    <meta charset=\"UTF-8\">\n"
         << "    <title>" << code << " " << message << "</title>\n"
         << "    <style>\n"
         << "        body {\n"
         << "            font-family: Arial, sans-serif;\n"
         << "            margin: 0;\n"
         << "            padding: 0;\n"
         << "            display: flex;\n"
         << "            justify-content: center;\n"
         << "            align-items: center;\n"
         << "            min-height: 100vh;\n"
         << "            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
         << "        }\n"
         << "        .error-container {\n"
         << "            text-align: center;\n"
         << "            background: white;\n"
         << "            padding: 40px;\n"
         << "            border-radius: 10px;\n"
         << "            box-shadow: 0 10px 40px rgba(0,0,0,0.2);\n"
         << "        }\n"
         << "        h1 {\n"
         << "            font-size: 72px;\n"
         << "            margin: 0;\n"
         << "            color: #667eea;\n"
         << "        }\n"
         << "        h2 {\n"
         << "            font-size: 24px;\n"
         << "            margin: 10px 0;\n"
         << "            color: #333;\n"
         << "        }\n"
         << "        p {\n"
         << "            color: #666;\n"
         << "            margin: 20px 0;\n"
         << "        }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <div class=\"error-container\">\n"
         << "        <h1>" << code << "</h1>\n"
         << "        <h2>" << message << "</h2>\n"
         << "        <p>The server encountered an error and could not complete your request.</p>\n"
         << "    </div>\n"
         << "</body>\n"
         << "</html>";
    return html.str();
}

// Try to read custom error page from file
std::string readCustomErrorPage(const std::string &path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return "";

    std::ostringstream content;
    content << file.rdbuf();
    file.close();
    return content.str();
}

// Main method: errorResponse(code, config)
// config is a map where key is error code and value is path to custom error page
std::string errorResponse(int code, const std::map<int, std::string> &config)
{
    static std::map<int, std::string> errorMessages = getErrorMessages();

    // Check if there's a custom error page configured for this code
    std::map<int, std::string>::const_iterator it = config.find(code);
    if (it != config.end())
    {
        std::string customPage = readCustomErrorPage(it->second);
        if (!customPage.empty())
            return customPage;
    }

    // Fall back to default error page
    std::string message = "Unknown Error";
    std::map<int, std::string>::iterator msgIt = errorMessages.find(code);
    if (msgIt != errorMessages.end())
        message = msgIt->second;

    return generateDefaultHTML(code, message);
}