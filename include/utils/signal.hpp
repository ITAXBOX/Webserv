#ifndef SIGNAL_HPP
#define SIGNAL_HPP

class WebServer;

// Signal handling utilities
namespace SignalHandler
{
	void setup(WebServer* server);
	void handle(int signum);
}

#endif
