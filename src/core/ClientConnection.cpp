#include "core/ClientConnection.hpp"
#include "utils/Logger.hpp"
#include <unistd.h>

ClientConnection::ClientConnection(int fd, size_t maxBodySize)
    : _fd(fd), _state(READING), _shouldClose(false)
{
    _parser.setMaxBodySize(maxBodySize);
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

void ClientConnection::setShouldClose(bool close)
{
    _shouldClose = close;
}

bool ClientConnection::shouldClose() const
{
    return _shouldClose;
}

void ClientConnection::appendToWriteBuffer(const std::string& data)
{
    _writeBuffer += data;
}

void ClientConnection::clearWriteBuffer()
{
    _writeBuffer.clear();
}

HttpParser& ClientConnection::getParser()
{
    return _parser;
}
