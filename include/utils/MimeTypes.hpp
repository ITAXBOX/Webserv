#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <string>
#include <algorithm>
#include <cctype>
#include <map>

class MimeTypes
{
private:
    static std::string getExtension(const std::string &filePath);

public:
    static std::string getMimeType(const std::string &filePath);

    static std::string getTypeByExtension(const std::string &extension);
};

#endif