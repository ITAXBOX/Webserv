#include "http/HttpParser.hpp"
#include "http/IParseState.hpp"
#include "utils/Logger.hpp"
#include "utils/defines.hpp"

HttpParser::HttpParser()
	: _currentState(new ParseRequestLineState()),
	  _maxBodySize(MAX_BODY_SIZE), // Default from defines.hpp
	  _isComplete(false),
	  _hasError(false),
	  _errorMessage("")
{
	Logger::debug("HttpParser created");
}

HttpParser::~HttpParser()
{
	delete _currentState;
	Logger::debug("HttpParser destroyed");
}

void HttpParser::parse(const std::string &data)
{
	if (_isComplete || _hasError)
		return; // Already finished parsing

	// Append new data to buffer
	_buffer += data;

	// Let current state handle the parsing
	_currentState->parse(*this);
}

bool HttpParser::isComplete() const
{
	return _isComplete;
}

bool HttpParser::hasError() const
{
	return _hasError;
}

std::string HttpParser::getErrorMessage() const
{
	return _errorMessage;
}

HttpRequest &HttpParser::getRequest()
{
	return _request;
}

const HttpRequest &HttpParser::getRequest() const
{
	return _request;
}

void HttpParser::reset()
{
	Logger::debug("Resetting HttpParser");
	
	_request.clear();
	_buffer.clear();
	_isComplete = false;
	_hasError = false;
	_errorMessage.clear();

	// Reset to initial state
	delete _currentState;
	_currentState = new ParseRequestLineState();
}

void HttpParser::setState(IParseState *newState)
{
	Logger::debug("State transition: " + _currentState->getName() + " -> " + newState->getName());
	delete _currentState;
	_currentState = newState;
}

void HttpParser::setComplete()
{
	_isComplete = true;
	Logger::info("HTTP request parsing complete");
}

void HttpParser::setError(const std::string &message)
{
	_hasError = true;
	_errorMessage = message;
	Logger::error("HTTP parsing error: " + message);
}
