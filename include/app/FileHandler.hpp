#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <string>
#include <ctime>

class FileHandler
{
private:
    FileHandler();
    ~FileHandler();
public:
    static bool fileExists(const std::string &path);
    static bool isDirectory(const std::string &path);
    static bool isReadable(const std::string &path);
    static std::string readFile(const std::string &path);
    static size_t getFileSize(const std::string &path);
};

#endif