#include "utils/utils.hpp"
#include "utils/Logger.hpp"
#include "utils/FileHandler.hpp"
#include <cctype>
#include <sys/stat.h>

// ============================================================================
// Webserv Startup Print
// ============================================================================

void printWebservStartup()
{
	std::cout << "===============================================" << std::endl;
	std::cout << "🔥  WEB SERV INITIALIZATION SEQUENCE STARTED  🔥" << std::endl;
	std::cout << "===============================================" << std::endl;
	std::cout << "     Team: Cache Me If You Can" << std::endl;
	std::cout << "     Engineers: fel-khat & mal-moha & aitawi" << std::endl;
	std::cout << std::endl;
}

// ============================================================================
// String Utilities
// ============================================================================

std::string trim(const std::string &str)
{
	size_t start = 0;
	size_t end = str.size();

	while (start < end && std::isspace(static_cast<unsigned char>(str[start])))
		start++;

	while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
		end--;

	return str.substr(start, end - start);
}

size_t findCRLF(const std::string &str, size_t start)
{
	return str.find("\r\n", start);
}

// ============================================================================
// Config Parsing Utilities
// ============================================================================

// Skip whitespace (space, tab, newline) and update line counter
void skipWhitespace(const std::string &input, size_t &pos, int &line)
{
	while (pos < input.length())
	{
		char c = input[pos];
		if (c == ' ' || c == '\t' || c == '\r')
			pos++;
		else if (c == '\n')
		{
			pos++;
			line++;
		}
		else
			break;
	}
}

// Skip comment (from # to end of line)
void skipComment(const std::string &input, size_t &pos)
{
	if (pos < input.length() && input[pos] == '#')
	{
		// Skip until newline or EOF
		while (pos < input.length() && input[pos] != '\n')
			pos++;
	}
}

// Check if character is valid for a word (alphanumeric, -, _, ., /, :)
bool isWordChar(char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) || 
	       c == '-' || c == '_' || c == '.' || c == '/' || c == ':';
}

// ============================================================================
// HTTP Utilities
// ============================================================================

HttpMethod stringToHttpMethod(const std::string &method)
{
	if (method == "GET")
		return HTTP_GET;
	if (method == "POST")
		return HTTP_POST;
	if (method == "DELETE")
		return HTTP_DELETE;
	if (method == "PUT")
		return HTTP_PUT;
	if (method == "HEAD")
		return HTTP_HEAD;
	return HTTP_UNKNOWN;
}

// ============================================================================
// APP Utilities
// ============================================================================

std::string buildFilePath(const std::string &uri, const std::string &rootDir, const std::string &defaultIndex)
{
	std::string filePath = rootDir;

	if (uri == "/" || uri.empty())
		filePath += "/" + defaultIndex;
	else
	{
		filePath += uri;

		if (FileHandler::isDirectory(filePath))
		{
			if (filePath[filePath.length() - 1] != '/')
				filePath += "/";
			filePath += defaultIndex;
		}
	}

	return filePath;
}
