#ifndef POLLER_HPP
#define POLLER_HPP

#include "utils/Logger.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <cerrno>

// Simple event structure - what happened on which fd
struct PollEvent
{
	int fd;
	bool readable; // Can read without blocking
	bool writable; // Can write without blocking
	bool error;	   // Error occurred
	bool hangup;   // Connection closed
};

class Poller
{
public:
	Poller();
	~Poller();

	// Add file descriptor to monitor
	// events: combination of EPOLLIN (read) and EPOLLOUT (write)
	bool addFd(int fd, int events);

	// Modify events for existing fd
	bool modifyFd(int fd, int events);

	// Remove fd from monitoring
	bool removeFd(int fd);

	// Wait for events (blocking)
	// timeout_ms: -1 for infinite, 0 for non-blocking, >0 for timeout
	// Returns number of events ready
	int wait(int timeout_ms = -1);

	// Get events that occurred (call after wait())
	const std::vector<PollEvent> &getEvents() const;

	// Helper: check if poller is valid
	bool isValid() const
	{
		return _epollFd >= 0;
	}

private:
	int _epollFd;								// epoll file descriptor
	std::vector<struct epoll_event> _rawEvents; // Raw epoll events
	std::vector<PollEvent> _events;				// Converted events for user

	void convertEvents(int count);

	Poller(const Poller &);
	Poller &operator=(const Poller &);
};

#endif
