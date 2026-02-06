#include "utils/SessionManager.hpp"

SessionManager *SessionManager::_instance = NULL;

SessionManager::SessionManager() : SESSION_TIMEOUT(1800)
{
    std::srand(std::time(0));
}

SessionManager *SessionManager::getInstance()
{
    if (!_instance)
        _instance = new SessionManager();
    return _instance;
}

std::string SessionManager::generateSessionId()
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::string id;
    for (int i = 0; i < 32; ++i)
        id += alphanum[std::rand() % (sizeof(alphanum) - 1)];
    return id;
}

std::string SessionManager::createSession()
{
    std::string id = generateSessionId();
    Session session;
    session.id = id;
    session.lastAccessed = std::time(0);
    _sessions[id] = session;
    Logger::debug("Created new session: " + id);
    return id;
}

Session *SessionManager::getSession(const std::string &id)
{
    std::map<std::string, Session>::iterator it = _sessions.find(id);
    if (it != _sessions.end())
    {
        // Check expiry
        if (std::difftime(std::time(0), it->second.lastAccessed) > SESSION_TIMEOUT)
        {
            Logger::debug("Session expired: " + id);
            _sessions.erase(it);
            return NULL;
        }

        it->second.lastAccessed = std::time(0);
        return &(it->second);
    }
    return NULL;
}

void SessionManager::destroySession(const std::string &id)
{
    std::map<std::string, Session>::iterator it = _sessions.find(id);
    if (it != _sessions.end())
    {
        Logger::debug("Destroying session: " + id);
        _sessions.erase(it);
    }
}

void SessionManager::setSessionData(const std::string &id, const std::string &key, const std::string &value)
{
    Session *session = getSession(id);
    if (session)
        session->data[key] = value;
}
