#ifndef CORE_HPP
#define CORE_HPP

#include <string>
#include <vector>

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
	std::string _configFile;
	bool _initialized;

	bool loadConfiguration();
	bool setupDefaultServer();
	void cleanup();

	WebServer(const WebServer &);
	WebServer &operator=(const WebServer &);
};

#endif