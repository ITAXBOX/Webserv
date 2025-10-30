#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "http/HttpRequest.hpp"
#include <string>

// Forward declarations
class IParseState;

// HttpParser: State Machine for parsing HTTP requests
// Uses State Pattern for clean separation of parsing stages
class HttpParser
{
public:
	HttpParser();
	~HttpParser();

	// Feed data to parser (can be called multiple times for chunked data)
	void parse(const std::string &data);

	// Check parsing status
	bool isComplete() const;
	bool hasError() const;
	std::string getErrorMessage() const;

	// Get the parsed request (only valid when isComplete() is true)
	HttpRequest &getRequest();
	const HttpRequest &getRequest() const;

	// Reset parser for reuse (e.g., for keep-alive connections)
	void reset();

private:
	HttpRequest _request;		// The request being built
	IParseState *_currentState; // Current parsing state
	std::string _buffer;		// Accumulated unparsed data
	bool _isComplete;			// Parsing completed successfully
	bool _hasError;				// Parsing error occurred
	std::string _errorMessage;	// Error description

	// State transitions (called by state objects)
	void setState(IParseState *newState);
	void setComplete();
	void setError(const std::string &message);

	// Allow states to access private members
	// friend is used here to grant access to private members
	// which means we don't have to expose setters publicly
	friend class ParseRequestLineState;
	friend class ParseHeadersState;
	friend class ParseBodyState;
	friend class ParseCompleteState;
	friend class ParseErrorState;

	// Non-copyable
	HttpParser(const HttpParser &);
	HttpParser &operator=(const HttpParser &);
};

#endif
