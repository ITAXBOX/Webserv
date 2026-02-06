#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "app/IMethodHandler.hpp"
#include "app/SessionHandler.hpp"
#include "app/DeleteHandler.hpp"
#include "app/PostHandler.hpp"
#include "app/HeadHandler.hpp"
#include "app/GetHandler.hpp"
#include "app/PutHandler.hpp"
#include "config/LocationConfig.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/defines.hpp"
#include "utils/Logger.hpp"
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
		const HttpRequest &request,
		const LocationConfig &location);

private:
	// Strategy map: HTTP method -> Handler
	std::map<HttpMethod, IMethodHandler *> handlers;

	// Initialize default handlers (GET, HEAD, POST, PUT, DELETE)
	void initializeDefaultHandlers();

	// Clean up all registered handlers
	void cleanup();

	// Non-copyable (handlers have ownership semantics)
	RequestHandler(const RequestHandler &);
	RequestHandler &operator=(const RequestHandler &);
};

#endif