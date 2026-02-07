#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include "core/core.hpp"
#include "utils/Logger.hpp"
#include <csignal>

class WebServer;

// Signal handling utilities
namespace SignalHandler
{
	void setup(WebServer *server);
}

#endif
