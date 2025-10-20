#include "../../include/utils/utils.hpp"
#include "../../include/utils/Logger.hpp"

void printWebservStartup(int argc, char **argv)
{
	std::cout << "==========================================" << std::endl;
	std::cout << "         🚀 Webserv starting up...         " << std::endl;
	std::cout << "==========================================" << std::endl;

	if (argc > 1)
        Logger::info(std::string("Configuration file: ") + argv[1]);
    else
        Logger::warn("No configuration file provided. Falling back to defaults.");
}
