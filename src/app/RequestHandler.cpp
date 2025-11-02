#include "app/RequestHandler.hpp"

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
	handlers[HTTP_GET] = new GetHandler();
	handlers[HTTP_HEAD] = new HeadHandler();
	handlers[HTTP_POST] = new PostHandler();
	handlers[HTTP_PUT] = new PutHandler();
	handlers[HTTP_DELETE] = new DeleteHandler();

	Logger::debug("Registered 5 default method handlers");
}

void RequestHandler::cleanup()
{
	for (std::map<HttpMethod, IMethodHandler *>::iterator it = handlers.begin(); it != handlers.end(); it++)
		delete it->second;
	handlers.clear();
}

void RequestHandler::registerHandler(HttpMethod method, IMethodHandler *handler)
{
	// If handler already exists for this method, delete it first
	if (handlers.count(method))
		delete handlers[method];

	handlers[method] = handler;
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
	std::map<HttpMethod, IMethodHandler *>::iterator it = handlers.find(method);

	if (it != handlers.end())
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
