#include "core/core.hpp"
#include "utils/Logger.hpp"
#include "utils/signal.hpp"
#include "utils/utils.hpp"

int main(int argc, char **argv)
{
	// Initialize logger settings
	Logger::init();

	// Print startup banner
	printWebservStartup();

	if (argc != 2)
	{
		Logger::error(std::string("Usage: ") + argv[0] + " <config_file>");
		return (1);
	}

	std::string configFile = argv[1];

	// Create and initialize WebServer (Facade pattern)
	WebServer server;

	// Setup signal handlers
	SignalHandler::setup(&server);

	// Initialize server with config file
	if (!server.init(configFile))
	{
		Logger::error("WebServer initialization failed");
		return (1);
	}

	// Start the server loop
	server.run();

	Logger::shutdown();

	return (0);
}
