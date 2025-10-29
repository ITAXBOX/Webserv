#include "http/IParseState.hpp"
#include "http/HttpParser.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include <sstream>
#include <cstdlib>

// ============================================================================
// ParseRequestLineState - Parse "GET /path HTTP/1.1"
// ============================================================================

void ParseRequestLineState::parse(HttpParser &parser)
{
	size_t pos = findCRLF(parser._buffer);
	if (pos == std::string::npos)
		return; // Need more data

	// Extract request line
	std::string line = parser._buffer.substr(0, pos);
	parser._buffer.erase(0, pos + 2); // Remove line + \r\n

	Logger::debug("Parsing request line: " + line);

	if (!parseRequestLine(parser, line))
	{
		parser.setState(new ParseErrorState());
		parser.setError("Invalid request line: " + line);
		return;
	}

	// Transition to headers state
	parser.setState(new ParseHeadersState());
	
	// Continue parsing if buffer has data
	if (!parser._buffer.empty())
		parser._currentState->parse(parser);
}

bool ParseRequestLineState::parseRequestLine(HttpParser &parser, const std::string &line)
{
	// Format: METHOD URI VERSION
	// Example: GET /index.html HTTP/1.1

	std::istringstream iss(line);
	std::string method, uri, version;

	if (!(iss >> method >> uri >> version))
		return false;

	// Set method
	HttpMethod httpMethod = stringToHttpMethod(method);
	if (httpMethod == HTTP_UNKNOWN)
	{
		Logger::warn("Unknown HTTP method: " + method);
		return false;
	}

	parser._request.setMethod(httpMethod);
	parser._request.setUri(uri);
	parser._request.setVersion(version);

	// Validate HTTP version
	if (version != "HTTP/1.1" && version != "HTTP/1.0")
	{
		Logger::warn("Unsupported HTTP version: " + version);
		// Continue anyway for compatibility
	}

	Logger::debug("Request: " + method + " " + uri + " " + version);
	return true;
}

// ============================================================================
// ParseHeadersState - Parse "Key: Value" headers
// ============================================================================

void ParseHeadersState::parse(HttpParser &parser)
{
	while (true)
	{
		size_t pos = findCRLF(parser._buffer);
		if (pos == std::string::npos)
			return; // Need more data

		std::string line = parser._buffer.substr(0, pos);
		parser._buffer.erase(0, pos + 2); // Remove line + \r\n

		// Empty line = end of headers
		if (line.empty())
		{
			Logger::debug("Headers parsing complete");

			// Check if we need to parse body
			size_t contentLength = getContentLength(parser);
			if (contentLength > 0)
			{
				Logger::debug("Content-Length detected, transitioning to body parsing");
				parser.setState(new ParseBodyState(contentLength));
				
				// Continue parsing body if buffer has data
				if (!parser._buffer.empty())
					parser._currentState->parse(parser);
			}
			else
			{
				// No body, parsing complete
				parser.setState(new ParseCompleteState());
				parser.setComplete();
			}
			return;
		}

		// Parse header line
		if (!parseHeaderLine(parser, line))
		{
			parser.setState(new ParseErrorState());
			parser.setError("Invalid header line: " + line);
			return;
		}
	}
}

bool ParseHeadersState::parseHeaderLine(HttpParser &parser, const std::string &line)
{
	// Format: Key: Value
	// Example: Host: localhost

	size_t colonPos = line.find(':');
	if (colonPos == std::string::npos)
		return false;

	std::string key = trim(line.substr(0, colonPos));
	std::string value = trim(line.substr(colonPos + 1));

	if (key.empty())
		return false;

	parser._request.addHeader(key, value);
	Logger::debug("Header: " + key + ": " + value);
	return true;
}

size_t ParseHeadersState::getContentLength(HttpParser &parser)
{
	std::string clHeader = parser._request.getHeader("Content-Length");
	if (clHeader.empty())
		return 0;

	return static_cast<size_t>(std::atoi(clHeader.c_str()));
}

// ============================================================================
// ParseBodyState - Parse request body
// ============================================================================

ParseBodyState::ParseBodyState(size_t contentLength)
	: _contentLength(contentLength), _bytesRead(0)
{
	Logger::debug("ParseBodyState created for body parsing");
}

void ParseBodyState::parse(HttpParser &parser)
{
	// Read up to Content-Length bytes
	size_t remaining = _contentLength - _bytesRead;
	size_t available = parser._buffer.size();
	size_t toRead = (available < remaining) ? available : remaining;

	if (toRead > 0)
	{
		// Append to body
		std::string currentBody = parser._request.getBody();
		currentBody.append(parser._buffer.substr(0, toRead));
		parser._request.setBody(currentBody);
		
		parser._buffer.erase(0, toRead);
		_bytesRead += toRead;

		std::ostringstream os;
		os << "Read " << toRead << " bytes, total: " << _bytesRead << "/" << _contentLength;
		Logger::debug(os.str());
	}

	// Check if body is complete
	if (_bytesRead >= _contentLength)
	{
		Logger::debug("Body parsing complete");
		parser.setState(new ParseCompleteState());
		parser.setComplete();
	}
}

// ============================================================================
// ParseCompleteState - Parsing finished successfully
// ============================================================================

void ParseCompleteState::parse(HttpParser &parser)
{
	(void)parser;
	// Do nothing - already complete
}

// ============================================================================
// ParseErrorState - Parsing error occurred
// ============================================================================

void ParseErrorState::parse(HttpParser &parser)
{
	(void)parser;
	// Do nothing - already in error state
}
