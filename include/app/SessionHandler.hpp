#ifndef SESSIONHANDLER_HPP
#define SESSIONHANDLER_HPP

#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "utils/SessionManager.hpp"
#include "utils/defines.hpp"
#include "utils/Logger.hpp"
#include <iostream>

class ClientConnection;

class SessionHandler
{
public:
    SessionHandler();
    ~SessionHandler();
    
    HttpResponse handle(const HttpRequest &request);
};

#endif
