#ifndef IMETHODHANDLER_HPP
#define IMETHODHANDLER_HPP

#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include <string>

// Strategy Pattern: IMethodHandler Interface
// Each HTTP method (GET, POST, DELETE, etc.) is a different strategy
// This allows easy addition of new methods without modifying existing code

class IMethodHandler
{
public:
	virtual ~IMethodHandler() {}

	// Handle the request and return a response
	virtual HttpResponse handle(
		const HttpRequest &request,
		const std::string &rootDir,
		const std::string &defaultIndex) = 0;

	// Get handler name for debugging
	virtual std::string getName() const = 0;
};

#endif