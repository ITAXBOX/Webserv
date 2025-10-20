#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

class Logger
{
public:
	enum Level
	{
		LEVEL_DEBUG = 0,
		LEVEL_INFO = 1,
		LEVEL_WARN = 2,
		LEVEL_ERROR = 3
	};

	static void setLevel(Level lvl);
	static void showTimestamp(bool enabled);

	static void debug(const std::string &msg);
	static void info(const std::string &msg);
	static void warn(const std::string &msg);
	static void error(const std::string &msg);

private:
	static void log(Level lvl, const std::string &msg);
	static const char *levelName(Level lvl);
	static std::string now();

private:
	static Level s_minLevel;
	static bool s_showTimestamp;
};

#endif