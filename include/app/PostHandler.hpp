#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include <string>
#include <map>

// PostHandler - Strategy for handling HTTP POST requests
// Handles form submissions and file uploads
// Supports: application/x-www-form-urlencoded, multipart/form-data

class PostHandler : public IMethodHandler
{
public:
	PostHandler();
	~PostHandler();

	// IMethodHandler interface implementation
	HttpResponse handle(
		const HttpRequest &request,
		const std::string &rootDir,
		const std::string &defaultIndex);

	std::string getName() const { return "POST"; }

private:
	// Parse form data
	std::map<std::string, std::string> parseFormData(const std::string &body);

	// Handle file upload (multipart/form-data)
	HttpResponse handleFileUpload(
		const HttpRequest &request,
		const std::string &rootDir);

	// Handle regular form submission
	HttpResponse handleFormSubmission(
		const HttpRequest &request,
		const std::string &rootDir);

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