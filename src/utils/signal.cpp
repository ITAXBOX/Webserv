#include "utils/signal.hpp"
#include "core/core.hpp"
#include "utils/Logger.hpp"
#include <csignal>

namespace SignalHandler
{
	// Global pointer for signal handler
	static WebServer* g_server = NULL;

	extern "C" void signalHandlerFunction(int)
	{
		if (g_server)
		{
			Logger::info("Received interrupt signal (Ctrl+C)");
			g_server->stop();
		}
	}

	void setup(WebServer* server)
	{
		g_server = server;
		
		// Setup signal handlers
		signal(SIGINT, signalHandlerFunction);  // Ctrl+C for graceful shutdown
		signal(SIGTSTP, SIG_IGN);               // Ignore Ctrl+Z to prevent suspension
		signal(SIGQUIT, SIG_IGN);               // Ignore Ctrl+\ to prevent core dump
	}

	void handle(int signum)
	{
		signalHandlerFunction(signum);
	}
}
