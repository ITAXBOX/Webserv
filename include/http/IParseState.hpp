#ifndef IPARSESTATE_HPP
#define IPARSESTATE_HPP

#include <string>

// Forward declaration
class HttpParser;

// IParseState: Interface for parser states (State Pattern)
// The State Pattern allows the parser to change its behavior
// Its one of the best ways to implement state machines.
// Each concrete state handles one phase of HTTP parsing
class IParseState
{
public:
	virtual ~IParseState()
	{
	}

	// Process data in this state
	virtual void parse(HttpParser &parser) = 0;

	// Get state name for debugging
	virtual std::string getName() const = 0;
};

// Concrete States

// State 1: Parse the request line (GET /path HTTP/1.1)
// Example: "GET /index.html HTTP/1.1"
// In this state, we read the first line of the HTTP request,
// extract the method, URI, and version, and store them in the HttpRequest object.
class ParseRequestLineState : public IParseState
{
public:
	virtual void parse(HttpParser &parser);
	virtual std::string getName() const
	{
		return "ParseRequestLine";
	}

private:
	bool parseRequestLine(HttpParser &parser, const std::string &line);
};

// State 2: Parse headers (Host: localhost, Content-Length: 100, etc.)
// Example: "Host: localhost"
// In this state, we read each header line, parse the key-value pairs,
// and store them in the HttpRequest object's headers map.
class ParseHeadersState : public IParseState
{
public:
	virtual void parse(HttpParser &parser);
	virtual std::string getName() const
	{
		return "ParseHeaders";
	}

private:
	bool parseHeaderLine(HttpParser &parser, const std::string &line);
	size_t getContentLength(HttpParser &parser);
};

// State 3: Parse body (POST/PUT data)
// Example: "field1=value1&field2=value2"
// In this state, we read the request body based on the Content-Length header
class ParseBodyState : public IParseState
{
public:
	virtual void parse(HttpParser &parser);
	virtual std::string getName() const
	{
		return "ParseBody";
	}

private:
	size_t _contentLength;
	size_t _bytesRead;

public:
	ParseBodyState(size_t contentLength);
};

// State 4: Parsing complete
// In this state, the entire HTTP request has been successfully parsed.
class ParseCompleteState : public IParseState
{
public:
	virtual void parse(HttpParser &parser);
	virtual std::string getName() const
	{
		return "ParseComplete";
	}
};

// State 5: Parsing error
// In this state, an error occurred during parsing.
class ParseErrorState : public IParseState
{
public:
	virtual void parse(HttpParser &parser);
	virtual std::string getName() const
	{
		return "ParseError";
	}
};

#endif
