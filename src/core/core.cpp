#include "core/core.hpp"
#include "core/EventLoop.hpp"
#include "core/ServerSocket.hpp"
#include "utils/Logger.hpp"

WebServer::WebServer()
	: _eventLoop(NULL), _configFile(""), _initialized(false)
{
	Logger::debug("WebServer facade created");
}

WebServer::~WebServer()
{
	cleanup();
	Logger::debug("WebServer facade destroyed");
}

bool WebServer::init(const std::string &configFile)
{
	if (_initialized)
	{
		Logger::warn("WebServer already initialized");
		return true;
	}

	_configFile = configFile;

	// Load configuration (for now, just use defaults)
	if (!loadConfiguration())
	{
		Logger::error("Failed to load configuration");
		return false;
	}

	// Create event loop
	_eventLoop = new EventLoop();
	if (!_eventLoop)
	{
		Logger::error("Failed to create EventLoop");
		return false;
	}

	// Setup servers (for now, just default server)
	if (!setupDefaultServer())
	{
		Logger::error("Failed to setup servers");
		cleanup();
		return false;
	}

	_initialized = true;
	Logger::info("WebServer initialized successfully");
	return true;
}

void WebServer::run()
{
	if (!_initialized)
	{
		Logger::error("WebServer not initialized. Call init() first.");
		return;
	}

	Logger::info("Starting WebServer...");
	_eventLoop->run();
}

void WebServer::stop()
{
	if (_eventLoop)
	{
		Logger::info("Stopping WebServer...");
		_eventLoop->stop();
	}
}

bool WebServer::loadConfiguration()
{
	if (_configFile.empty())
	{
		Logger::warn("No configuration file provided. Using defaults.");
		return true;
	}

	Logger::info(std::string("Loading configuration from: ") + _configFile);
	// TODO: Parse config file in Phase 2
	// For now, just acknowledge the file
	return true;
}

bool WebServer::setupDefaultServer()
{
	// Create a default server listening on 0.0.0.0:8080
	ServerSocket *srv = new ServerSocket();
	if (!srv)
	{
		Logger::error("Failed to allocate ServerSocket");
		return false;
	}

	if (!srv->init("0.0.0.0", 8080, 128))
	{
		Logger::error("Failed to initialize default server on 0.0.0.0:8080");
		delete srv;
		return false;
	}

	_servers.push_back(srv);
	_eventLoop->addServer(srv);

	Logger::info("Default server configured: 0.0.0.0:8080");
	return true;
}

void WebServer::cleanup()
{
	Logger::debug("Cleaning up WebServer resources");

	// Delete event loop (it will clean up its own resources)
	if (_eventLoop)
	{
		delete _eventLoop;
		_eventLoop = NULL;
	}

	// Note: ServerSockets are now owned and deleted by EventLoop
	// So we just clear our reference list
	_servers.clear();

	_initialized = false;
}
