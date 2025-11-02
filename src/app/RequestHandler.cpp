#include "app/RequestHandler.hpp"
#include "app/GetHandler.hpp"
// #include "app/HeadHandler.hpp"
#include "app/PostHandler.hpp"
// #include "app/PutHandler.hpp"
#include "app/DeleteHandler.hpp"
#include "utils/Logger.hpp"
#include "utils/StatusCodes.hpp"
#include "utils/utils.hpp"
#include "utils/defines.hpp"

RequestHandler::RequestHandler()
{
	Logger::debug("RequestHandler created with Strategy Pattern");
	initializeDefaultHandlers();
}

RequestHandler::~RequestHandler()
{
	cleanup();
	Logger::debug("RequestHandler destroyed");
}

void RequestHandler::initializeDefaultHandlers()
{
	// Register default handlers for each HTTP method
	_handlers[HTTP_GET] = new GetHandler();
	// _handlers[HTTP_HEAD] = new HeadHandler();
	_handlers[HTTP_POST] = new PostHandler();
	// _handlers[HTTP_PUT] = new PutHandler();
	_handlers[HTTP_DELETE] = new DeleteHandler();

	Logger::debug("Registered 5 default method handlers");
}

void RequestHandler::cleanup()
{
	for (std::map<HttpMethod, IMethodHandler *>::iterator it = _handlers.begin(); it != _handlers.end(); it++)
		delete it->second;
	_handlers.clear();
}

void RequestHandler::registerHandler(HttpMethod method, IMethodHandler *handler)
{
	// If handler already exists for this method, delete it first
	if (_handlers.count(method))
		delete _handlers[method];

	_handlers[method] = handler;
	Logger::debug("Registered custom handler for method: " + toString(static_cast<int>(method)));
}

HttpResponse RequestHandler::handleRequest(
	const HttpRequest &request,
	const std::string &rootDir,
	const std::string &defaultIndex)
{
	HttpMethod method = request.getMethod();
	std::string methodStr = request.getMethodString();

	Logger::info("RequestHandler routing: " + methodStr + " " + request.getUri());

	// Find the appropriate handler (Strategy)
	std::map<HttpMethod, IMethodHandler *>::iterator it = _handlers.find(method);

	if (it != _handlers.end())
	{
		// Delegate to the strategy
		IMethodHandler *handler = it->second;
		Logger::debug("Delegating to " + handler->getName() + " handler");
		return handler->handle(request, rootDir, defaultIndex);
	}

	// No handler found for this method
	Logger::warn("No handler registered for method: " + methodStr);
	return StatusCodes::createErrorResponse(HTTP_METHOD_NOT_ALLOWED, "Method Not Allowed");
}
