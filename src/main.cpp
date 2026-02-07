#include "core/core.hpp"
#include "utils/Logger.hpp"
#include "utils/signal.hpp"
#include "utils/utils.hpp"

int main(int argc, char **argv)
{
	Logger::showTimestamp(true);
	Logger::showColors(true);

	printWebservStartup();

	if (argc != 2)
	{
		Logger::error(std::string("Usage: ") + argv[0] + " <config_file>");
		return (1);
	}

	std::string configFile = argv[1];

	// Create and initialize WebServer (Facade pattern)
	WebServer server;

	// Setup signal handlers (Ctrl+C, Ctrl+Z, Ctrl+\)
	SignalHandler::setup(&server);

	if (!server.init(configFile))
	{
		Logger::error("WebServer initialization failed");
		return (1);
	}

	server.run();

	Logger::info("Server shutdown complete");
	return (0);
}
