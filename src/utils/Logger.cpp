#include "utils/Logger.hpp"

// if LOGGER_DEBUG macro is defined in the build, set default log level to DEBUG, else INFO
Logger::Level Logger::s_minLevel =
#ifdef LOGGER_DEBUG
	Logger::LEVEL_DEBUG;
#else
	Logger::LEVEL_INFO;
#endif

bool Logger::s_showTimestamp = true;
bool Logger::s_showColors = true;

void Logger::init()
{
	showTimestamp(true);
	showColors(true);
}

void Logger::showTimestamp(bool enabled)
{
	s_showTimestamp = enabled;
}

void Logger::showColors(bool enabled)
{
	s_showColors = enabled;
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

void Logger::shutdown()
{
	log(LEVEL_INFO, "Server shutdown complete");
}

void Logger::log(Logger::Level lvl, const std::string &msg)
{
	// the minimum log level that should be printed.
	if (lvl < s_minLevel)
		return;

	// build a string piece by piece (like a mini "formatter") instead of concatenating multiple strings.
	std::ostringstream line;

	if (s_showColors)
		line << levelColor(lvl);

	if (s_showTimestamp)
	{
		line << "[" << now() << "] ";
	}
	line << levelName(lvl) << " " << msg;

	if (s_showColors)
		line << "\033[0m"; // Reset color

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

const char *Logger::levelColor(Logger::Level lvl)
{
	switch (lvl)
	{
	case LEVEL_DEBUG:
		return "\033[36m"; // Cyan
	case LEVEL_INFO:
		return "\033[32m"; // Green
	case LEVEL_WARN:
		return "\033[33m"; // Yellow
	case LEVEL_ERROR:
		return "\033[31m"; // Red
	}
	return "\033[0m"; // Reset
}

// Get current timestamp as string in format "YYYY-MM-DD HH:MM:SS"
std::string Logger::now()
{
	std::time_t t = std::time(0);
	std::tm *lt = std::localtime(&t);
	char buf[32];
	if (lt && std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt))
		return std::string(buf);
	return "0000-00-00 00:00:00";
}

std::string Logger::errnoMsg(const std::string &prefix)
{
	std::ostringstream os;
	os << prefix << ": " << std::strerror(errno);
	return os.str();
}

// Helper to format a message with a file descriptor (e.g. "Failed to read from socket (fd=5)")
std::string Logger::fdMsg(const std::string &prefix, int fd)
{
	std::ostringstream os;
	os << prefix << " (fd=" << fd << ")";
	return os.str();
}

// Helper to format a message with a file descriptor and additional details
std::string Logger::connMsg(const std::string &prefix, int fd, const std::string &detail)
{
	std::ostringstream os;
	os << prefix << " (fd=" << fd << ")";
	if (!detail.empty())
		os << " - " << detail;
	return os.str();
}
