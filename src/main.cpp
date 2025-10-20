#include "../include/webserv.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
		return 1;
	}

	printWebservStartup(argc, argv);

	// Placeholder for server initialization and startup logic

	std::cout << "Server initialized successfully (placeholder)." << std::endl;

	return 0;
}
