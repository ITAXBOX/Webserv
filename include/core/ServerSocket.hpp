#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

#include <string>

class ServerSocket
{
public:
	ServerSocket();
	~ServerSocket();

	bool init(const std::string &ip, int port, int backlog);
	int acceptClient();
	void close();

	int fd() const
	{
		return _fd;
	}
	bool valid() const
	{
		return _fd >= 0;
	}
	std::string ip() const
	{
		return _ip;
	}
	int port() const
	{
		return _port;
	}

private:
	ServerSocket(const ServerSocket &);
	ServerSocket &operator=(const ServerSocket &);

	bool createSocket();
	bool applySocketOptions();
	bool setNonBlocking(int fd);
	bool buildSockAddr(const std::string &ip, int port, struct sockaddr_in &out);
	bool bindSocket(const struct sockaddr_in &addr);
	bool startListening(int backlog);
	void closeAndReset();
	void logListening(const std::string &ip, int port) const;

private:
	int _fd;
	std::string _ip;
	int _port;
};

#endif