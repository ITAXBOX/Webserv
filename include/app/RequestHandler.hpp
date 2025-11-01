#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include <string>
#include <map>

// RequestHandler - Context (Strategy Pattern)
// Routes HTTP requests to appropriate method handlers (strategies)
// Each HTTP method has its own handler implementing IMethodHandler interface
// Allows runtime registration of custom handlers for extensibility

class RequestHandler
{
public:
	RequestHandler();
	~RequestHandler();
	
	// Main request handler - delegates to registered strategy
	HttpResponse handleRequest(
		const HttpRequest& request,
		const std::string& rootDir,
		const std::string& defaultIndex
	);
	
	// Register custom handler for specific HTTP method
	// Takes ownership of the handler pointer
	void registerHandler(HttpMethod method, IMethodHandler* handler);

private:
	// Strategy map: HTTP method -> Handler
	std::map<HttpMethod, IMethodHandler*> _handlers;
	
	// Initialize default handlers (GET, HEAD, POST, PUT, DELETE)
	void initializeDefaultHandlers();
	
	// Clean up all registered handlers
	void cleanup();
	
	// Non-copyable (handlers have ownership semantics)
	RequestHandler(const RequestHandler&);
	RequestHandler& operator=(const RequestHandler&);
};

#endif