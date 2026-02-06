#ifndef CORE_HPP
#define CORE_HPP

#include "config/ServerConfig.hpp"
#include "config/ConfigParser.hpp"
#include "config/Tokenizer.hpp"
#include "core/EventLoop.hpp"
#include "core/ServerSocket.hpp"
#include "utils/MimeTypes.hpp"
#include "utils/Logger.hpp"
#include "utils/utils.hpp"
#include "utils/defines.hpp"
#include <csignal>
#include <fstream>

class EventLoop;
class ServerSocket;

// Facade pattern: simplifies interaction with the complex subsystem
// Main only needs to call WebServer::run() without knowing about EventLoop, ServerSocket, etc.
class WebServer
{
public:
	WebServer();
	~WebServer();

	// Initialize from config file (or use defaults if empty)
	bool init(const std::string &configFile = "");

	// Start the server (blocking call)
	void run();

	// Stop the server gracefully
	void stop();

private:
	EventLoop *_eventLoop;
	std::vector<ServerSocket *> _servers;
	std::vector<ServerConfig> _serverConfigs;
	std::string _configFile;
	bool _initialized;

	bool loadConfiguration();
	bool setupServers();
	void cleanup();

	WebServer(const WebServer &);
	WebServer &operator=(const WebServer &);
};

#endif