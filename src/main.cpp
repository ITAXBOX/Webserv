#include "../include/webserv.hpp"

int main(int argc, char **argv)
{
	Logger::showTimestamp(true);

	if (argc > 2)
	{
		Logger::error(std::string("Usage: ") + argv[0] + " [config_file]");
		return 1;
	}

	if (argc > 1)
		Logger::info(std::string("Configuration file: ") + argv[1]);
	else
		Logger::warn("No configuration file provided. Falling back to defaults.");

	ServerSocket srv;
	if (!srv.init("0.0.0.0", 8080, 128))
	{
		Logger::error("Server socket initialization failed.");
		return 1;
	}

	Logger::info("Core ready (placeholder). Event loop will be added next.");

	return 0;
}
