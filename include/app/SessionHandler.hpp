#ifndef SESSIONHANDLER_HPP
#define SESSIONHANDLER_HPP

#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

class ClientConnection;

class SessionHandler
{
public:
    SessionHandler();
    ~SessionHandler();
    
    HttpResponse handle(const HttpRequest &request);
};

#endif
