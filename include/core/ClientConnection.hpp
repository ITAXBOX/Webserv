#ifndef CLIENTCONNECTION_HPP
# define CLIENTCONNECTION_HPP

#include "http/HttpParser.hpp"
#include "core/CgiState.hpp"
#include "utils/Logger.hpp"
#include <unistd.h>

enum ConnState { READING, WRITING, CLOSED, CGI_ACTIVE };

class ClientConnection
{
private:
    int _fd; // socket fd for this client
    std::string _readBuffer; // store data read from client
    std::string _writeBuffer; // store data to be sent to client
    ConnState _state;
    bool _shouldClose;
    HttpParser _parser; // HTTP request parser
    CgiState  _cgiState;
public:
    ClientConnection(int fd, size_t maxBodySize);
    ~ClientConnection();

    int getFd() const;
    std::string& getReadBuffer();
    std::string& getWriteBuffer();
    ConnState getState() const;
    void setState(ConnState state);
    
    void setShouldClose(bool close);
    bool shouldClose() const;

    void appendToWriteBuffer(const std::string& data);
    void clearWriteBuffer();
    
    // HTTP Parser access
    HttpParser& getParser();

    // CGI State
    CgiState& getCgiState() { return _cgiState; }
};

#endif