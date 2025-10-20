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

	ServerSocket *srv = new ServerSocket();
	if (!srv->init("0.0.0.0", 8080, 128))
	{
		Logger::error("Server socket initialization failed.");
		delete srv;
		return 1;
	}

	Logger::info("Server socket initialized on 0.0.0.0:8080");

	EventLoop loop;
	loop.addServer(srv);
	loop.run();

	Logger::info("Server stopped. Cleaning up...");

	return 0;
}
