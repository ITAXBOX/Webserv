#include "app/PostHandler.hpp"
#include "app/CgiExecutor.hpp"
#include "utils/FileHandler.hpp"
#include <sstream>

HttpResponse PostHandler::handle(
	const HttpRequest &request,
    const LocationConfig &location)
{
    std::string rootDir = location.getRoot();
    if (rootDir.empty()) rootDir = DEFAULT_ROOT;
    std::string defaultIndex = DEFAULT_INDEX;
    if (!location.getIndex().empty()) defaultIndex = location.getIndex()[0];

	std::string uri = request.getUri();
    Logger::info("PostHandler processing: " + uri);

    // Normalize URI (remove query strings, fragments)
    size_t endPos = uri.find_first_of("?#");
    if (endPos != std::string::npos)
        uri = uri.substr(0, endPos);

    // Build file path (Simple manual construction for now)
    std::string filePath = rootDir;
    if (!filePath.empty() && filePath[filePath.size() - 1] != '/' && !uri.empty() && uri[0] != '/')
        filePath += "/";
    filePath += uri;

    // Handle Directory Index
    if (FileHandler::isDirectory(filePath))
    {
        if (filePath[filePath.size() - 1] != '/')
            filePath += "/";
        filePath += defaultIndex;
    }

	// Check for CGI
	if (isCgiRequest(filePath, location))
		return executeCgi(request, filePath, location);

	// Check if request has a body
	if (request.getHeader("Content-Length") == "")
	{
		Logger::warn("POST request without Content-Length header");
		return StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
	}

	// Get content type
	std::string contentType = request.getHeader("Content-Type");

	// Handle multipart/form-data (file uploads)
	if (contentType.find("multipart/form-data") != std::string::npos)
		return handleFileUpload(request, location);

	// Handle application/x-www-form-urlencoded (regular forms)
	else if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
		return handleFormSubmission(request, rootDir);

	// Unsupported content type
	else
	{
		Logger::warn("Unsupported Content-Type: " + contentType);
		return StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
	}
}

HttpResponse PostHandler::handleFormSubmission(
	const HttpRequest &request,
	const std::string &rootDir)
{
	(void)rootDir;

	std::string body = request.getBody();
	Logger::debug("Form data received: " + body);

	std::map<std::string, std::string> formData = parseFormData(body);

	for (std::map<std::string, std::string>::iterator it = formData.begin(); it != formData.end(); it++)
		Logger::debug("Form field: " + it->first + " = " + it->second);

	std::ostringstream html;
	html << "<html><head><title>Form Submitted</title></head><body>";
	html << "<h1>Form Submission Successful</h1>";
	html << "<h2>Received Data:</h2>";
	html << "<ul>";

	for (std::map<std::string, std::string>::iterator it = formData.begin(); it != formData.end(); it++)
		html << "<li><strong>" << it->first << ":</strong> " << it->second << "</li>";

	html << "</ul>";
	html << "<a href=\"/\">Back to Home</a>";
	html << "</body></html>";

	std::string responseBody = html.str();

	HttpResponse response;
	response.setStatus(HTTP_OK, "OK")
		.addHeader("Content-Type", "text/html")
		.addHeader("Content-Length", toString(responseBody.size()))
		.setBody(responseBody);

	Logger::info("Form submission processed successfully");
	return response;
}

HttpResponse PostHandler::handleFileUpload(
	const HttpRequest &request,
	const LocationConfig &location)
{
    std::string rootDir = location.getRoot();
    if (rootDir.empty()) rootDir = DEFAULT_ROOT;

	Logger::info("Processing file upload");

	std::string body = request.getBody();
	std::string contentType = request.getHeader("Content-Type");

	// Extract boundary from Content-Type header
	// Example: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW
	size_t boundaryPos = contentType.find("boundary=");
	if (boundaryPos == std::string::npos)
	{
		Logger::error("No boundary found in multipart/form-data");
		return StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
	}

	std::string boundary = "--" + contentType.substr(boundaryPos + 9);
	Logger::debug("Boundary: " + boundary);

	// Find file content between boundaries
	size_t fileStart = body.find("\r\n\r\n");
	if (fileStart == std::string::npos)
	{
		Logger::error("Invalid multipart format");
		return StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
	}
	fileStart += 4; // Skip \r\n\r\n

	size_t fileEnd = body.find(boundary, fileStart);
	if (fileEnd == std::string::npos)
	{
		Logger::error("File end boundary not found");
		return StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
	}

	// Extract filename from Content-Disposition header
	size_t filenamePos = body.find("filename=\"");
	if (filenamePos == std::string::npos)
	{
		Logger::error("Filename not found in upload");
		return StatusCodes::createErrorResponse(HTTP_BAD_REQUEST, "Bad Request");
	}
	filenamePos += 10; // Skip filename="

	size_t filenameEnd = body.find("\"", filenamePos);
	std::string filename = body.substr(filenamePos, filenameEnd - filenamePos);

	// Extract file content
	std::string fileContent = body.substr(fileStart, fileEnd - fileStart - 2); // -2 for \r\n

	// Use configured upload path or default to root/uploads
	std::string uploadDir = location.getUploadPath();
    if (uploadDir.empty())
        uploadDir = rootDir + "/uploads";

    // Check if directory exists (we are not allowed to mkdir)
    if (!FileHandler::isDirectory(uploadDir))
    {
         Logger::error("Upload directory does not exist: " + uploadDir);
         return StatusCodes::createErrorResponse(HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error");
    }

	// Save file
	bool saved = saveUploadedFile(filename, fileContent, uploadDir);

	// Generate response
	std::string responseBody = generateUploadResponse(filename, fileContent.size(), saved);

	HttpResponse response;
	if (saved)
	{
		response.setStatus(HTTP_OK, "OK");
		Logger::info("File uploaded successfully: " + filename);
	}
	else
	{
		response.setStatus(HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error");
		Logger::error("Failed to save uploaded file: " + filename);
	}

	response.addHeader("Content-Type", "text/html")
		.addHeader("Content-Length", toString(responseBody.size()))
		.setBody(responseBody);

	return response;
}

std::map<std::string, std::string> PostHandler::parseFormData(const std::string &body)
{
	std::map<std::string, std::string> data;

	// Split by '&'
	size_t start = 0;
	size_t ampPos = 0;

	while ((ampPos = body.find('&', start)) != std::string::npos)
	{
		std::string pair = body.substr(start, ampPos - start);

		// Split by '='
		size_t eqPos = pair.find('=');
		if (eqPos != std::string::npos)
		{
			std::string key = pair.substr(0, eqPos);
			std::string value = pair.substr(eqPos + 1);

			// URL decode (basic - replace + with space)
			for (size_t i = 0; i < value.length(); i++)
			{
				if (value[i] == '+')
					value[i] = ' ';
			}

			data[key] = value;
		}

		start = ampPos + 1;
	}

	// Handle last pair
	if (start < body.length())
	{
		std::string pair = body.substr(start);
		size_t eqPos = pair.find('=');
		if (eqPos != std::string::npos)
		{
			std::string key = pair.substr(0, eqPos);
			std::string value = pair.substr(eqPos + 1);

			for (size_t i = 0; i < value.length(); i++)
			{
				if (value[i] == '+')
					value[i] = ' ';
			}

			data[key] = value;
		}
	}

	return data;
}

bool PostHandler::saveUploadedFile(
	const std::string &filename,
	const std::string &content,
	const std::string &uploadDir)
{
	// NOTE: Upload directory must already exist
	// Build full file path
	std::string filePath = uploadDir + "/" + filename;

	// Write file
	std::ofstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		Logger::error("Failed to open file for writing: " + filePath);
		return false;
	}

	file.write(content.c_str(), content.size());
	file.close();

	Logger::info("File saved: " + filePath + " (" + toString(content.size()) + " bytes)");
	return true;
}

std::string PostHandler::generateUploadResponse(
	const std::string &filename,
	size_t fileSize,
	bool success)
{
	std::ostringstream html;
	html << "<html><head><title>File Upload Result</title></head><body>";

	if (success)
	{
		html << "<h1>File Uploaded Successfully!</h1>";
		html << "<p><strong>Filename:</strong> " << filename << "</p>";
		html << "<p><strong>Size:</strong> " << fileSize << " bytes</p>";
		html << "<p>File saved to: /uploads/" << filename << "</p>";
	}
	else
	{
		html << "<h1>Upload Failed</h1>";
		html << "<p>Failed to save file: " << filename << "</p>";
	}

	html << "<br><a href=\"/\">Back to Home</a>";
	html << "</body></html>";

	return html.str();
}



