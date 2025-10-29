#include "app/app.hpp"
#include <algorithm>
#include <cctype>

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

    // Text types
    if (ext == "html" || ext == "htm")
        return "text/html";
    if (ext == "css")
        return "text/css";
    if (ext == "txt")
        return "text/plain";
    if (ext == "csv")
        return "text/csv";
    if (ext == "xml")
        return "text/xml";

    // Application types
    if (ext == "js" || ext == "mjs")
        return "application/javascript";
    if (ext == "json")
        return "application/json";
    if (ext == "pdf")
        return "application/pdf";
    if (ext == "zip")
        return "application/zip";
    if (ext == "gz" || ext == "gzip")
        return "application/gzip";
    if (ext == "tar")
        return "application/x-tar";
    if (ext == "7z")
        return "application/x-7z-compressed";
    if (ext == "rar")
        return "application/vnd.rar";
    if (ext == "doc")
        return "application/msword";
    if (ext == "docx")
        return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if (ext == "xls")
        return "application/vnd.ms-excel";
    if (ext == "xlsx")
        return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    if (ext == "ppt")
        return "application/vnd.ms-powerpoint";
    if (ext == "pptx")
        return "application/vnd.openxmlformats-officedocument.presentationml.presentation";

    // Image types
    if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    if (ext == "png")
        return "image/png";
    if (ext == "gif")
        return "image/gif";
    if (ext == "svg")
        return "image/svg+xml";
    if (ext == "ico")
        return "image/x-icon";
    if (ext == "webp")
        return "image/webp";
    if (ext == "bmp")
        return "image/bmp";
    if (ext == "tiff" || ext == "tif")
        return "image/tiff";

    // Audio types
    if (ext == "mp3")
        return "audio/mpeg";
    if (ext == "wav")
        return "audio/wav";
    if (ext == "ogg")
        return "audio/ogg";
    if (ext == "aac")
        return "audio/aac";
    if (ext == "flac")
        return "audio/flac";

    // Video types
    if (ext == "mp4")
        return "video/mp4";
    if (ext == "mpeg" || ext == "mpg")
        return "video/mpeg";
    if (ext == "webm")
        return "video/webm";
    if (ext == "avi")
        return "video/x-msvideo";
    if (ext == "mov")
        return "video/quicktime";
    if (ext == "wmv")
        return "video/x-ms-wmv";

    // Font types
    if (ext == "woff")
        return "font/woff";
    if (ext == "woff2")
        return "font/woff2";
    if (ext == "ttf")
        return "font/ttf";
    if (ext == "otf")
        return "font/otf";

    // Default
    return "application/octet-stream";
}

// Get MIME type based on file path
std::string MimeTypes::getMimeType(const std::string &filePath)
{
    std::string ext = getExtension(filePath);
    return getTypeByExtension(ext);
}
