// TCP server socket implementation
// Handles creating, binding, listening, and accepting client connections
// This is the part of a program that waits for incoming network connections, for example:
// when you connect to a web server, there’s a socket like this listening for you.

// A socket is a file descriptor (number) that represents a communication channel.
// In C (and by extension C++), a socket is just a number you read from and write to,
// like a file, but it connects over a network.
#include "core/ServerSocket.hpp"

ServerSocket::ServerSocket()
	: _fd(-1), _ip(""), _port(-1)
{
}

ServerSocket::~ServerSocket()
{
	if (_fd != -1)
		close(_fd);
}

bool ServerSocket::setNonBlocking(int fd)
{
	// The non-blocking behavior is achieved by explicitly setting the socket file descriptor
	// (both the server and the accepted clients) to non-blocking mode using the fcntl() system call.
	// F_GETFL Command to get file descriptor flags - which are: O_RDONLY, O_WRONLY, O_RDWR, etc.
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		Logger::error(std::string("fcntl(F_GETFL) failed: ") + std::strerror(errno));
		return false;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		Logger::error(std::string("fcntl(F_SETFL, O_NONBLOCK) failed: ") + std::strerror(errno));
		return false;
	}
	return true;
}

bool ServerSocket::init(const std::string &ip, int port, int backlog)
{
	if (isValid())
	{
		Logger::warn("ServerSocket already initialized; closing previous fd");
		closeAndReset();
	}

	_ip = ip;
	_port = port;

	if (!createSocket())
		return false;
	if (!applySocketOptions())
	{
		closeAndReset();
		return false;
	}
	if (!setNonBlocking(_fd))
	{
		closeAndReset();
		return false;
	}

	struct sockaddr_in addr;
	if (!buildSockAddr(_ip, _port, addr))
	{
		closeAndReset();
		return false;
	}
	if (!bindSocket(addr))
	{
		closeAndReset();
		return false;
	}
	if (!startListening(backlog))
	{
		closeAndReset();
		return false;
	}

	logListening(_ip, _port);
	return true;
}

bool ServerSocket::createSocket()
{
	// AF_INET: IPv4, SOCK_STREAM: TCP, 0: default protocol (TCP for SOCK_STREAM)
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
	{
		Logger::error(std::string("socket() failed: ") + std::strerror(errno));
		return false;
	}
	return true;
}

bool ServerSocket::applySocketOptions()
{
	int yes = 1;
	// _fd: The file descriptor of the socket to configure.
	// SOL_SOCKET: Specifies that the option is at the socket API level (not protocol level like TCP).
	// SO_REUSEADDR: The specific option to enable. It allows the socket to bind to a port that is still in TIME_WAIT from a previous run.
	// &yes: A pointer to the value enabling the option (integer 1).
	// sizeof(yes): The length of the option value data.
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		Logger::error(std::string("setsockopt(SO_REUSEADDR) failed: ") + std::strerror(errno));
		return false;
	}
	return true;
}

// sockaddr_in is a struct that holds an internet address
// out contains in its struct: sin_family, sin_port, sin_addr
bool ServerSocket::buildSockAddr(const std::string &ip, int port, struct sockaddr_in &out)
{
	std::memset(&out, 0, sizeof(out));
	// out.sin_family: Address family (AF_INET for IPv4)
	out.sin_family = AF_INET;
	// out.sin_port: Port number (in network byte order)
	// the htons function converts a port number in host byte order to network byte order
	out.sin_port = htons(static_cast<unsigned short>(port));

	// INADDR_ANY if empty, "*", or "0.0.0.0"
	if (ip.empty() || ip == "*" || ip == "0.0.0.0")
	{
		// out.sin_addr.s_addr: IP address (in network byte order)
		// htonl converts a long integer from host byte order to network byte order
		// INADDR_ANY is a constant that means "any local IP address"
		out.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		// inet_addr: Convert IPv4 address from string to binary form
		unsigned long a = inet_addr(ip.c_str());
		// INADDR_NONE indicates invalid address
		if (a == INADDR_NONE)
		{
			Logger::error("Invalid IPv4 address: " + ip);
			return false;
		}
		out.sin_addr.s_addr = a;
	}
	return true;
}

bool ServerSocket::bindSocket(const struct sockaddr_in &addr)
{
	// bind the socket to the specified IP address and port
	// sockaddr_in is cast to sockaddr for the bind function
	// reinterpret_cast<const struct sockaddr *>(&addr) is used for this purpose
	// it tells the compiler to treat the pointer to sockaddr_in as a pointer to sockaddr
	// dynamically converting the type without changing the actual data
	if (bind(_fd, reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr)) < 0)
	{
		std::ostringstream os;
		os << "bind(" << (_ip.empty() ? "0.0.0.0" : _ip) << ":" << _port
		   << ") failed: " << std::strerror(errno);
		Logger::error(os.str());
		return false;
	}
	return true;
}

bool ServerSocket::startListening(int backlog)
{
	// listen for incoming connections on the socket
	// backlog specifies the maximum number of pending connections
	if (listen(_fd, backlog) < 0)
	{
		Logger::error(std::string("listen() failed: ") + std::strerror(errno));
		return false;
	}
	return true;
}

void ServerSocket::closeAndReset()
{
	if (_fd >= 0)
		close(_fd);
	_fd = -1;
}

void ServerSocket::logListening(const std::string &ip, int port) const
{
	std::ostringstream os;
	os << "Socket ready: " << (ip.empty() ? "0.0.0.0" : ip) << ":" << port
	   << " (fd=" << _fd << ", non-blocking)";
	Logger::info(os.str());
}

int ServerSocket::acceptClient()
{
	if (!isValid())
		return -1;

	// sockaddr_in cli will hold the client's address info
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	// accept a new client connection
	// sockaddr_in is cast to sockaddr for the accept function
	// reinterpret_cast<struct sockaddr *>(&cli) is used for this purpose
	int cfd = accept(_fd, reinterpret_cast<struct sockaddr *>(&cli), &len);
	if (cfd < 0)
	{
		// EAGAIN or EWOULDBLOCK means no pending connections (non-blocking mode)
		// errno is a global variable set by system calls in the event of an error
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return -1;
		Logger::error(std::string("accept() failed: ") + std::strerror(errno));
		return -1;
	}

	// Make client non-blocking too
	if (!setNonBlocking(cfd))
	{
		close(cfd);
		return -1;
	}
	return cfd;
}
