#ifndef ERRORPAGEGENERATOR_HPP
#define ERRORPAGEGENERATOR_HPP

#include <string>
#include <map>

class ErrorPageGenerator
{
public:
    // Get singleton instance
    static ErrorPageGenerator &getInstance();

    // Delete copy constructor and assignment operator
    ErrorPageGenerator(const ErrorPageGenerator &);
    ErrorPageGenerator &operator=(const ErrorPageGenerator &);

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
    ErrorPageGenerator() {}
    ~ErrorPageGenerator() {}

    // Generate default error page HTML
    std::string generateDefaultPage(int statusCode, const std::string &reason) const;
};

#endif