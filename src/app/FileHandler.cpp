#include "app/FileHandler.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>

FileHandler::FileHandler() {}

FileHandler::~FileHandler() {}

bool FileHandler::fileExists(const std::string &path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool FileHandler::isDirectory(const std::string &path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0)
        return false;
    return S_ISDIR(buffer.st_mode);
}

bool FileHandler::isReadable(const std::string &path)
{
    std::ifstream file(path.c_str());
    return file.good();
}

std::string FileHandler::readFile(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file)
        return "";
    
    std::ostringstream content;
    content << file.rdbuf();
    file.close();
    return content.str();
}

size_t FileHandler::getFileSize(const std::string &path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0)
        return 0;
    return static_cast<size_t>(buffer.st_size);
}
