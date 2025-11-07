#ifndef ERRORPAGEHANDLER_HPP
#define ERRORPAGEHANDLER_HPP

#include <string>
#include <map>

class ErrorPageHandler
{
public:
    // Get singleton instance
    static ErrorPageHandler &getInstance();

    // Delete copy constructor and assignment operator
    ErrorPageHandler(const ErrorPageHandler &) = delete;
    ErrorPageHandler &operator=(const ErrorPageHandler &) = delete;

    // Configure custom error pages
    void setErrorPage(int statusCode, const std::string &filePath);
    void clearErrorPage(int statusCode);
    void clearAllErrorPages();

    // Get error page content
    std::string getErrorPage(int statusCode, const std::string &reason) const;

    // Check if custom error page exists for status code
    bool hasCustomPage(int statusCode) const;

private:
    std::map<int, std::string> errorPages_;

    // Private constructor for singleton
    ErrorPageHandler() {}
    ~ErrorPageHandler() {}

    // Generate default error page HTML
    std::string generateDefaultPage(int statusCode, const std::string &reason) const;
};

#endif