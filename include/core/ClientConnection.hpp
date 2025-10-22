#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

#include <string>

enum ConnState { READING, WRITING, CLOSED };

class ClientConnection
{
private:
    int _fd; // socket fd for this client
    std::string _readBuffer; // store data read from client
    std::string _writeBuffer; // store data to be sent to client
    ConnState _state;
public:
    ClientConnection(int fd);
    ~ClientConnection();

    int getFd() const;
    std::string& getReadBuffer();
    std::string& getWriteBuffer();
    ConnState getState() const;
    void setState(ConnState state);

    void appendToWriteBuffer(const std::string& data);
    void clearWriteBuffer();
};

#endif