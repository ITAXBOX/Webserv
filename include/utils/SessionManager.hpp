#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include "utils/Logger.hpp"
#include <cstdlib>
#include <sstream>
#include <string>
#include <ctime>
#include <map>

// Simple session storage
struct Session
{
    std::string id;
    std::map<std::string, std::string> data;
    std::time_t lastAccessed;
};

class SessionManager
{
private:
    std::map<std::string, Session> _sessions;
    static SessionManager *_instance;
    const int SESSION_TIMEOUT; // 30 minutes in seconds

    SessionManager();
    std::string generateSessionId();

public:
    static SessionManager *getInstance();

    // Create new session
    std::string createSession();

    // Get session by ID, updating last accessed time
    Session *getSession(const std::string &id);

    // Destroy a specific session
    void destroySession(const std::string &id);

    // Add data to session
    void setSessionData(const std::string &id, const std::string &key, const std::string &value);
};

#endif
