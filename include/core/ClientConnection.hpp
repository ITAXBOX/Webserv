#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

#include "http/HttpParser.hpp"

enum ConnState { READING, WRITING, CLOSED };

class ClientConnection
{
private:
    int _fd; // socket fd for this client
    std::string _readBuffer; // store data read from client
    std::string _writeBuffer; // store data to be sent to client
    ConnState _state;
    HttpParser _parser; // HTTP request parser
public:
    ClientConnection(int fd, size_t maxBodySize);
    ~ClientConnection();

    int getFd() const;
    std::string& getReadBuffer();
    std::string& getWriteBuffer();
    ConnState getState() const;
    void setState(ConnState state);

    void appendToWriteBuffer(const std::string& data);
    void clearWriteBuffer();
    
    // HTTP Parser access
    HttpParser& getParser();
};

#endif