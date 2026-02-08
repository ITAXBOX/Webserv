#include "core/core.hpp"

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

	// Load configuration
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

	// Setup servers (from config or defaults)
	if (!setupServers())
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
		Logger::error("No configuration file provided.");
		return false;
	}

	Logger::info(std::string("Loading configuration from: ") + _configFile);

	// Read config file
	std::ifstream file(_configFile.c_str());
	if (!file.is_open())
	{
		Logger::error("Failed to open config file: " + _configFile);
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();
	file.close();

	if (content.empty())
	{
		Logger::error("Config file is empty: " + _configFile);
		return false;
	}

	// Tokenize
	Tokenizer tokenizer;
	if (!tokenizer.tokenize(content))
	{
		Logger::error("Config tokenization failed: " + tokenizer.getError());
		return false;
	}

	Logger::debug("Config tokenization successful");

	// Parse
	ConfigParser parser;
	if (!parser.parse(tokenizer.getTokens()))
	{
		Logger::error("Config parsing failed: " + parser.getError());
		return false;
	}

	_serverConfigs = parser.getServers();
	Logger::info("Config parsed successfully: " + toString(_serverConfigs.size()) + " server(s)");
	return true;
}

bool WebServer::setupServers()
{
	// If no config loaded, return error
	if (_serverConfigs.empty())
	{
		Logger::error("No server configurations found in file.");
		return false;
	}

	// Create servers from config
	for (size_t i = 0; i < _serverConfigs.size(); i++)
	{
		const ServerConfig &config = _serverConfigs[i];

		std::string host = config.getHost();
		if (host.empty())
			host = DEFAULT_HOST;

		int port = config.getPort();
		if (port == 0)
			port = DEFAULT_PORT;

		ServerSocket *srv = new ServerSocket();
		if (!srv)
		{
			Logger::error("Failed to allocate ServerSocket");
			return false;
		}

		if (!srv->init(host, port, DEFAULT_BACKLOG))
		{
			Logger::error("Failed to initialize server on " + host + ":" + toString(port));
			delete srv;
			return false;
		}

		_servers.push_back(srv);
		_eventLoop->addServer(srv, config);

		// Log server names if any
		const std::vector<std::string> &names = config.getServerNames();
		if (!names.empty())
		{
			std::string nameList = "";
			for (size_t j = 0; j < names.size(); j++)
			{
				nameList += names[j];
				if (j < names.size() - 1)
					nameList += ", ";
			}
			Logger::info("Server configured: " + host + ":" + toString(port) + " (" + nameList + ")");
		}
		else
			Logger::info("Server configured: " + host + ":" + toString(port));
	}

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
