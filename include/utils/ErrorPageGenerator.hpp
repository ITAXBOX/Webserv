#ifndef ERRORPAGEGENERATOR_HPP
#define ERRORPAGEGENERATOR_HPP

#include "utils/FileHandler.hpp"
#include <sstream>
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

    void clearErrorPage(int statusCode);
    void clearAllErrorPages();

    // Get error page content
    std::string getErrorPage(int statusCode, const std::string &reason) const;

private:
    std::map<int, std::string> errorPages_;

    // Private constructor for singleton
    ErrorPageGenerator() {}
    ~ErrorPageGenerator() {}

    // Generate default error page HTML
    std::string generateDefaultPage() const;
};

#endif