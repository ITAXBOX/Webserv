#ifndef ERRORPAGEGENERATOR_HPP
#define ERRORPAGEGENERATOR_HPP

#include <string>
#include <map>
#include <sstream>
#include <fstream>

class ErrorPageGenerator
{
private:
    static std::map<int, std::string> getErrorMessages();
    static std::string generateDefaultHTML(int code, const std::string &message);
    static std::string readCustomErrorPage(const std::string &path);

public:
    static std::string errorResponse(int code, const std::map<int, std::string> &config);
};

#endif