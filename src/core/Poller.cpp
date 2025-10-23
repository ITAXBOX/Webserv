#include "core/Poller.hpp"
#include "utils/Logger.hpp"
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sstream>

// epoll is Linux-specific and much more efficient than poll()
// It uses a red-black tree internally for O(1) operations
// Perfect for handling thousands of connections
Poller::Poller()
	: _epollFd(-1)
{
	// Create epoll instance
	// EPOLL_CLOEXEC: close on exec (security best practice)
	_epollFd = epoll_create1(EPOLL_CLOEXEC);
	if (_epollFd < 0)
	{
		Logger::error(Logger::errnoMsg("epoll_create1() failed"));
		return;
	}

	// Start with 64, will grow if needed
	_rawEvents.reserve(64);
	_events.reserve(64);

	Logger::debug(Logger::fdMsg("Poller created with epoll", _epollFd));
}

Poller::~Poller()
{
	if (_epollFd >= 0)
	{
		close(_epollFd);
		Logger::debug("Poller destroyed");
	}
}

bool Poller::addFd(int fd, int events)
{
	if (!isValid())
	{
		Logger::error("Poller is not valid (epoll_create1 failed?)");
		return false;
	}

	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	// Edge-triggered
	ev.events = events | EPOLLET;
	ev.data.fd = fd;

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) < 0)
	{
		std::ostringstream os;
		os << "epoll_ctl(ADD, fd=" << fd << ") failed";
		Logger::error(Logger::errnoMsg(os.str()));
		return false;
	}

	Logger::debug(Logger::fdMsg("Added fd to poller", fd));
	return true;
}

bool Poller::modifyFd(int fd, int events)
{
	if (!isValid())
		return false;

	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	// Edge-triggered
	ev.events = events | EPOLLET;
	ev.data.fd = fd;

	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) < 0)
	{
		std::ostringstream os;
		os << "epoll_ctl(MOD, fd=" << fd << ") failed";
		Logger::error(Logger::errnoMsg(os.str()));
		return false;
	}

	Logger::debug(Logger::fdMsg("Modified fd in poller", fd));
	return true;
}

bool Poller::removeFd(int fd)
{
	if (!isValid())
		return false;

	// In newer kernels, the event pointer can be NULL for EPOLL_CTL_DEL
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) < 0)
	{
		// Don't log error if fd is already closed (EBADF is normal)
		if (errno != EBADF)
		{
			std::ostringstream os;
			os << "epoll_ctl(DEL, fd=" << fd << ") failed";
			Logger::error(Logger::errnoMsg(os.str()));
		}
		return false;
	}

	Logger::debug(Logger::fdMsg("Removed fd from poller", fd));
	return true;
}

int Poller::wait(int timeout_ms)
{
	if (!isValid())
		return -1;

	// Resize to hold up to 64 events per call (good balance)
	_rawEvents.resize(64);

	// Wait for events
	int n = epoll_wait(_epollFd, _rawEvents.data(), _rawEvents.size(), timeout_ms);

	if (n < 0)
	{
		// EINTR is normal (signal interrupted), don't log error
		if (errno == EINTR)
		{
			Logger::debug("epoll_wait() interrupted by signal");
			return 0;
		}
		Logger::error(Logger::errnoMsg("epoll_wait() failed"));
		return -1;
	}

	if (n > 0)
	{
		std::ostringstream os;
		os << "epoll_wait() returned " << n << " event(s)";
		Logger::debug(os.str());
	}

	// Convert raw epoll events to our simple format
	convertEvents(n);
	return n;
}

const std::vector<PollEvent>& Poller::getEvents() const
{
	return _events;
}

void Poller::convertEvents(int count)
{
	_events.clear();

	for (int i = 0; i < count; i++)
	{
		const struct epoll_event& ev = _rawEvents[i];
		PollEvent pe;

		pe.fd = ev.data.fd;
		pe.readable = (ev.events & EPOLLIN) != 0;
		pe.writable = (ev.events & EPOLLOUT) != 0;
		pe.error = (ev.events & EPOLLERR) != 0;
		pe.hangup = (ev.events & (EPOLLHUP | EPOLLRDHUP)) != 0;

		_events.push_back(pe);

		// Debug log events
		std::ostringstream os;
		os << "Event on fd=" << pe.fd << ": ";
		if (pe.readable) os << "READ ";
		if (pe.writable) os << "WRITE ";
		if (pe.error) os << "ERROR ";
		if (pe.hangup) os << "HANGUP ";
		Logger::debug(os.str());
	}
}
