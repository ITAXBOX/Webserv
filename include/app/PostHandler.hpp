#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "app/BaseMethodHandler.hpp"
#include "utils/FileHandler.hpp"
#include "utils/utils.hpp"
#include <fstream>
#include <sstream>
#include <map>

// PostHandler - Strategy for handling HTTP POST requests
// Handles form submissions and file uploads
// Supports: application/x-www-form-urlencoded, multipart/form-data

class PostHandler : public BaseMethodHandler
{
public:
	PostHandler() {}
	~PostHandler() {}

	// IMethodHandler interface implementation
	HttpResponse handle(
		const HttpRequest &request,
		const LocationConfig &location);

	std::string getName() const { return "POST"; }

private:
	// Parse form data
	std::map<std::string, std::string> parseFormData(const std::string &body);

	// Handle file upload (multipart/form-data)
	HttpResponse handleFileUpload(
		const HttpRequest &request,
		const LocationConfig &location);

	// Handle regular form submission
	HttpResponse handleFormSubmission(const HttpRequest &request);

	// Save uploaded file
	bool saveUploadedFile(
		const std::string &filename,
		const std::string &content,
		const std::string &uploadDir);

	// Generate upload response HTML
	std::string generateUploadResponse(
		const std::string &filename,
		size_t fileSize,
		bool success);
};

#endif