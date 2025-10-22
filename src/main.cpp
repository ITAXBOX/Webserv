#include "../include/webserv.hpp"

int main(int argc, char **argv)
{
	Logger::showTimestamp(true);
	Logger::showColors(true);

	printWebservStartup();

	if (argc > 2)
	{
		Logger::error(std::string("Usage: ") + argv[0] + " [config_file]");
		return 1;
	}

	std::string configFile = (argc == 2) ? argv[1] : "";

	// Create and initialize WebServer (Facade pattern)
	WebServer server;
	if (!server.init(configFile))
	{
		Logger::error("WebServer initialization failed");
		return 1;
	}

	server.run();

	Logger::info("Server shutdown complete");
	return 0;
}
