#include "utils/signal.hpp"

namespace SignalHandler
{
	// Global pointer for signal handler
	static WebServer *g_server = NULL;

	extern "C" void signalHandlerFunction(int)
	{
		if (g_server)
		{
			Logger::info("Received interrupt signal (Ctrl+C)");
			g_server->stop();
		}
	}

	void setup(WebServer *server)
	{
		g_server = server;

		// Setup signal handlers
		signal(SIGINT, signalHandlerFunction); // Ctrl+C for graceful shutdown
		signal(SIGTSTP, SIG_IGN);			   // Ignore Ctrl+Z to prevent suspension
		signal(SIGQUIT, SIG_IGN);			   // Ignore Ctrl+\ to prevent core dump
		signal(SIGPIPE, SIG_IGN);			   // Ignore SIGPIPE (broken pipe on write)
	}

	void handle(int signum)
	{
		signalHandlerFunction(signum);
	}
}
