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

	int getFd() const
	{
		return _fd;
	}
	bool isValid() const
	{
		return _fd >= 0;
	}
	std::string getIp() const
	{
		return _ip;
	}
	int getPort() const
	{
		return _port;
	}

private:
	int _fd;
	std::string _ip;
	int _port;

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
};

#endif