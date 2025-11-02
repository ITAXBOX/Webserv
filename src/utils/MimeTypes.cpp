#include "utils/MimeTypes.hpp"

// Extract file extension from path
std::string MimeTypes::getExtension(const std::string &filePath)
{
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == filePath.length() - 1)
        return "";

    std::string ext = filePath.substr(dotPos + 1);

    // convert to lowercase for case-insensitive comparison
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return ext;
}

std::string MimeTypes::getTypeByExtension(const std::string &extension)
{
    std::string ext = extension;

    // convert to lowercase for case-insensitive comparison
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Static MIME type map
    static std::map<std::string, std::string> mimeMap;

    if (mimeMap.empty())
    {
        // Text types
        mimeMap["html"] = "text/html";
        mimeMap["htm"] = "text/html";
        mimeMap["css"] = "text/css";
        mimeMap["txt"] = "text/plain";
        mimeMap["csv"] = "text/csv";
        mimeMap["xml"] = "text/xml";

        // Application types
        mimeMap["js"] = "application/javascript";
        mimeMap["mjs"] = "application/javascript";
        mimeMap["json"] = "application/json";
        mimeMap["pdf"] = "application/pdf";
        mimeMap["zip"] = "application/zip";
        mimeMap["gz"] = "application/gzip";
        mimeMap["gzip"] = "application/gzip";
        mimeMap["tar"] = "application/x-tar";
        mimeMap["7z"] = "application/x-7z-compressed";
        mimeMap["rar"] = "application/vnd.rar";
        mimeMap["doc"] = "application/msword";
        mimeMap["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        mimeMap["xls"] = "application/vnd.ms-excel";
        mimeMap["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        mimeMap["ppt"] = "application/vnd.ms-powerpoint";
        mimeMap["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";

        // Image types
        mimeMap["jpg"] = "image/jpeg";
        mimeMap["jpeg"] = "image/jpeg";
        mimeMap["png"] = "image/png";
        mimeMap["gif"] = "image/gif";
        mimeMap["svg"] = "image/svg+xml";
        mimeMap["ico"] = "image/x-icon";
        mimeMap["webp"] = "image/webp";
        mimeMap["bmp"] = "image/bmp";
        mimeMap["tiff"] = "image/tiff";
        mimeMap["tif"] = "image/tiff";

        // Audio types
        mimeMap["mp3"] = "audio/mpeg";
        mimeMap["wav"] = "audio/wav";
        mimeMap["ogg"] = "audio/ogg";
        mimeMap["aac"] = "audio/aac";
        mimeMap["flac"] = "audio/flac";

        // Video types
        mimeMap["mp4"] = "video/mp4";
        mimeMap["mpeg"] = "video/mpeg";
        mimeMap["mpg"] = "video/mpeg";
        mimeMap["webm"] = "video/webm";
        mimeMap["avi"] = "video/x-msvideo";
        mimeMap["mov"] = "video/quicktime";
        mimeMap["wmv"] = "video/x-ms-wmv";

        // Font types
        mimeMap["woff"] = "font/woff";
        mimeMap["woff2"] = "font/woff2";
        mimeMap["ttf"] = "font/ttf";
        mimeMap["otf"] = "font/otf";
    }

    std::map<std::string, std::string>::const_iterator it = mimeMap.find(ext);
    if (it != mimeMap.end())
        return it->second;

    // Default type
    return "application/octet-stream";
}

// Get MIME type based on file path
std::string MimeTypes::getMimeType(const std::string &filePath)
{
    std::string ext = getExtension(filePath);
    return getTypeByExtension(ext);
}
