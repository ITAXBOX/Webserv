#include "utils/Logger.hpp"
#include <iostream>
#include <ctime>
#include <sstream>

Logger::Level Logger::s_minLevel =
#ifdef LOGGER_DEBUG
	Logger::LEVEL_DEBUG;
#else
	Logger::LEVEL_INFO;
#endif

bool Logger::s_showTimestamp = true;

void Logger::setLevel(Logger::Level lvl)
{
	s_minLevel = lvl;
}

void Logger::showTimestamp(bool enabled)
{
	s_showTimestamp = enabled;
}

void Logger::debug(const std::string &msg)
{
	log(LEVEL_DEBUG, msg);
}

void Logger::info(const std::string &msg)
{
	log(LEVEL_INFO, msg);
}

void Logger::warn(const std::string &msg)
{
	log(LEVEL_WARN, msg);
}

void Logger::error(const std::string &msg)
{
	log(LEVEL_ERROR, msg);
}

void Logger::log(Logger::Level lvl, const std::string &msg)
{
	// the minimum log level that should be printed.
	if (lvl < s_minLevel)
		return;

	// build a string piece by piece (like a mini “formatter”) instead of concatenating multiple strings.
	std::ostringstream line;
	if (s_showTimestamp)
	{
		line << "[" << now() << "] ";
	}
	line << levelName(lvl) << " " << msg;

	if (lvl == LEVEL_ERROR)
		std::cerr << line.str() << std::endl;
	else
		std::cout << line.str() << std::endl;
}

const char *Logger::levelName(Logger::Level lvl)
{
	switch (lvl)
	{
	case LEVEL_DEBUG:
		return "[DEBUG]";
	case LEVEL_INFO:
		return "[INFO]";
	case LEVEL_WARN:
		return "[WARN]";
	case LEVEL_ERROR:
		return "[ERROR]";
	}
	return "[UNKWN]";
}

std::string Logger::now()
{
	std::time_t t = std::time(0);
	std::tm *lt = std::localtime(&t);
	char buf[32];
	if (lt && std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt))
		return std::string(buf);
	return "0000-00-00 00:00:00";
}
