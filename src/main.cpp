#include "../include/webserv.hpp"

int main(int argc, char **argv)
{
	Logger::showTimestamp(true);

	if (argc > 2)
	{
		Logger::error(std::string("Usage: ") + argv[0] + " [config_file]");
		return 1;
	}

	printWebservStartup(argc, argv);

	// Placeholder for server initialization and startup logic

	Logger::debug("Initializing core components (placeholder)...");
	Logger::info("Server initialized successfully (placeholder).");

	return 0;
}
