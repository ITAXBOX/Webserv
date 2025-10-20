#include "../../include/utils/utils.hpp"

void printWebservStartup(int argc, char **argv)
{
	std::cout << "==========================================" << std::endl;
	std::cout << "         🚀 Webserv starting up...         " << std::endl;
	std::cout << "==========================================" << std::endl;

	if (argc > 1)
		std::cout << "Configuration file: " << argv[1] << std::endl;
	else
		std::cout << "Using default configuration file." << std::endl;
}
