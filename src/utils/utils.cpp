#include "../../include/utils/utils.hpp"
#include "../../include/utils/Logger.hpp"

void printWebservStartup(int argc, char **argv)
{
	std::cout << "\n===============================================" << std::endl;
	std::cout << "🔥  WEB SERV INITIALIZATION SEQUENCE STARTED  🔥" << std::endl;
	std::cout << "===============================================" << std::endl;
	std::cout << "     Team: Cache Me If You Can" << std::endl;
	std::cout << "     Engineers: mal-moha & aitawi" << std::endl;
	std::cout << std::endl;

	if (argc > 1)
		Logger::info(std::string("Configuration file: ") + argv[1]);
	else
		Logger::warn("No configuration file provided. Falling back to defaults.");
}
