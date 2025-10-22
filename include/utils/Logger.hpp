#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <sstream>

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
	static void showColors(bool enabled);

	static void debug(const std::string &msg);
	static void info(const std::string &msg);
	static void warn(const std::string &msg);
	static void error(const std::string &msg);

	static std::string errnoMsg(const std::string &prefix);
	static std::string fdMsg(const std::string &prefix, int fd);
	static std::string connMsg(const std::string &prefix, int fd, const std::string &detail = "");

private:
	static void log(Level lvl, const std::string &msg);
	static const char *levelName(Level lvl);
	static const char *levelColor(Level lvl);
	static std::string now();

private:
	static Level s_minLevel;
	static bool s_showTimestamp;
	static bool s_showColors;
};

#endif