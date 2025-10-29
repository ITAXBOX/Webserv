#ifndef APP_HPP
#define APP_HPP

#include <string>

class MimeTypes
{
private:
    static std::string getExtension(const std::string &filePath);

public:
    static std::string getMimeType(const std::string &filePath);

    static std::string getTypeByExtension(const std::string &extension);
};

#endif