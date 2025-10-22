#include "core/ClientConnection.hpp"
#include "utils/Logger.hpp"
#include <unistd.h>
#include <sstream>

ClientConnection::ClientConnection(int fd)
    : _fd(fd), _state(READING)
{
    Logger::debug(Logger::fdMsg("ClientConnection created", fd));
}

ClientConnection::~ClientConnection()
{
    Logger::debug(Logger::fdMsg("ClientConnection destroyed, closing socket", _fd));
    close(_fd);
}

int ClientConnection::getFd() const
{
    return _fd;
}

std::string& ClientConnection::getReadBuffer()
{
    return _readBuffer;
}

std::string& ClientConnection::getWriteBuffer()
{
    return _writeBuffer;
}

ConnState ClientConnection::getState() const
{
    return _state;
}

void ClientConnection::setState(ConnState state)
{
    _state = state;
}

void ClientConnection::appendToWriteBuffer(const std::string& data)
{
    _writeBuffer += data;
}

void ClientConnection::clearWriteBuffer()
{
    _writeBuffer.clear();
}