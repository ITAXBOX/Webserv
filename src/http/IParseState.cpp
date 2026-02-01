#include "http/IParseState.hpp"
#include "http/HttpParser.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include <sstream>
#include <cstdlib>

// ============================================================================
// ParseRequestLineState - Parse "GET /path HTTP/1.1"
// ============================================================================

// the CRLF is used to denote the end of a line in HTTP
// Its the Delimiter that tells us where one line ends and the next begins
// which is "\r\n" (carriage return + line feed)

void ParseRequestLineState::parse(HttpParser &parser)
{
	// we search for the first CRLF to get the request line
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

	// Extract components, we expect exactly 3 components
	// example: line = "GET /index.html HTTP/1.1"
	// method = "GET", uri = "/index.html", version = "HTTP/1.1"
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
		// when we reach an empty line, it means headers are done
		// and now for the body (if any)
		if (line.empty())
		{
			Logger::debug("Headers parsing complete");

			// Check Transfer-Encoding: chunked
			std::string te = parser.getRequest().getHeader("Transfer-Encoding");
			if (te.find("chunked") != std::string::npos)
			{
				Logger::debug("Transfer-Encoding: chunked detected");
				parser.setState(new ParseChunkedBodyState());
				if (!parser._buffer.empty())
					parser._currentState->parse(parser);
				return;
			}

			// Check if we need to parse body
			size_t contentLength = getContentLength(parser);
			if (contentLength > parser.getMaxBodySize())
			{
				Logger::warn("Request body too large: " + toString(contentLength));
				parser.setState(new ParseErrorState());
				parser.setError("Payload Too Large"); // Specific message for 413
				return;
			}

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

	// what we are doing here is converting the Content-Length header value to size_t
	return static_cast<size_t>(std::strtoul(clHeader.c_str(), NULL, 10));
}

// ============================================================================
// ParseBodyState - Parse request body
// ============================================================================

ParseBodyState::ParseBodyState(size_t contentLength)
	: _contentLength(contentLength), _bytesRead(0)
{
	Logger::debug("ParseBodyState created for body parsing");
}

// Parses the Http request body incrementally until we reach Content-Length
// bodies can be large and may arrive in chunks, like file uploads
// data arrive over the network in chunks, not all at once
void ParseBodyState::parse(HttpParser &parser)
{
	// Read up to Content-Length bytes
	// example: Content-Length = 1000
	// we may receive data in chunks of 200, 300, etc.
	// so we need to keep track of how much we've read so far
	// remaining = 1000 - 300 = 700 bytes  // Still need 700 more
	// available = 200 bytes                // Only have 200 right now
	// toRead = min(200, 700) = 200 bytes  // Read what we have (200)
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
// ParseChunkedBodyState - Parse chunked data
// ============================================================================

ParseChunkedBodyState::ParseChunkedBodyState()
	: _state(CHUNK_SIZE), _chunkSize(0), _chunkRead(0)
{
    Logger::debug("ParseChunkedBodyState created");
}

void ParseChunkedBodyState::parse(HttpParser &parser)
{
    while (true) // Process all available data
    {
        if (_state == CHUNK_SIZE)
        {
            size_t pos = findCRLF(parser._buffer);
            if (pos == std::string::npos)
                return; // Need more data for size line

            std::string line = parser._buffer.substr(0, pos);
            // Parse hex size
            std::stringstream ss(line);
            if (!(ss >> std::hex >> _chunkSize))
            {
                 parser.setState(new ParseErrorState());
                 parser.setError("Invalid chunk size: " + line);
                 return;
            }
            
            parser._buffer.erase(0, pos + 2); // Consume line + CRLF
            
            if (_chunkSize == 0)
            {
                _state = CHUNK_TRAILERS;
            }
            else
            {
                _state = CHUNK_DATA;
                _chunkRead = 0;
            }
        }
        else if (_state == CHUNK_DATA)
        {
            size_t remaining = _chunkSize - _chunkRead;
            if (remaining == 0) 
            {
                // Should not happen if transitions correct, but safety
                _state = CHUNK_DATA_CRLF;
                continue;
            }
            
            size_t available = parser._buffer.size();
            size_t toRead = (available < remaining) ? available : remaining;
            
            if (toRead == 0) return; // Need data
            
            std::string currentBody = parser._request.getBody();
            // Check limits!
            if (currentBody.size() + toRead > parser.getMaxBodySize())
            {
                 parser.setState(new ParseErrorState());
                 parser.setError("Payload Too Large");
                 return;
            }

            currentBody.append(parser._buffer.substr(0, toRead));
            parser._request.setBody(currentBody);
            
            parser._buffer.erase(0, toRead);
            _chunkRead += toRead;
            
            if (_chunkRead >= _chunkSize)
            {
                _state = CHUNK_DATA_CRLF;
            }
        }
        else if (_state == CHUNK_DATA_CRLF)
        {
            if (parser._buffer.length() < 2)
                return; // Need CRLF
            
            if (parser._buffer.substr(0, 2) != "\r\n")
            {
                 parser.setState(new ParseErrorState());
                 parser.setError("Invalid chunk terminator");
                 return;
            }
            parser._buffer.erase(0, 2);
            _state = CHUNK_SIZE;
        }
        else if (_state == CHUNK_TRAILERS)
        {
            // Simple: wait for empty line (CRLF) to end message
            // or consume headers until empty line
            size_t pos = findCRLF(parser._buffer);
            if (pos == std::string::npos)
                return;

            std::string line = parser._buffer.substr(0, pos);
            parser._buffer.erase(0, pos + 2);
            
            if (line.empty()) 
            {
                Logger::debug("Chunked parsing complete");
                parser.setState(new ParseCompleteState());
                parser.setComplete();
                return;
            }
            // else: skip trailer header
        }
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
