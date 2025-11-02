#include "app/PutHandler.hpp"

HttpResponse PutHandler::handle(
    const HttpRequest &request,
    const std::string &rootDir,
    const std::string &defaultIndex)
{
    (void)defaultIndex;

    std::string path = buildFilePath(request.getUri(), rootDir, "");

    Logger::info("PUT request for: " + path);

    // Extract the body (file content) from the HTTP request
    const std::string &body = request.getBody();

    // Try to open the target file for writing
    // - std::ios::trunc → overwrite existing file
    // - std::ios::binary → handle binary uploads safely
    std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!out)
    {
        Logger::error("Failed to open file for writing (" + path + "): " +
                      std::string(strerror(errno)));
        return StatusCodes::createErrorResponse(
            HTTP_INTERNAL_SERVER_ERROR, strerror(errno));
    }

    // Write the request body into the file
    out.write(body.c_str(), body.size());
    out.close();

    // Prepare the HTTP response
    HttpResponse res;

    // If the file already existed, respond with 200 OK
    // Otherwise, respond with 201 Created (new resource)
    if (FileHandler::fileExists(path))
        res.setStatus(HTTP_OK, "OK");
    else
        res.setStatus(HTTP_CREATED, "Created");

    res.addHeader("Content-Length", "0");

    return res;
}
