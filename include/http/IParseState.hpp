#ifndef IPARSESTATE_HPP
#define IPARSESTATE_HPP

#include <string>

// Forward declaration
class HttpParser;

// IParseState: Interface for parser states (State Pattern)
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
