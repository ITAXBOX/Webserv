#include "config/ServerConfig.hpp"
#include "utils/defines.hpp"

ServerConfig::ServerConfig()
	: host(DEFAULT_HOST),			// Listen on all interfaces by default
	  port(DEFAULT_PORT),			// Default port
	  root(DEFAULT_ROOT),			// Default web root
	  clientMaxBodySize(MAX_BODY_SIZE)	// 1MB default
{
	// Default index files
	index.push_back(DEFAULT_INDEX);
	index.push_back("index.htm");
	
	// Default server name
	serverNames.push_back("localhost");
}

ServerConfig::~ServerConfig()
{
}

// Setters (Builder pattern)
ServerConfig &ServerConfig::setHost(const std::string &h)
{
	host = h;
	return *this;
}

ServerConfig &ServerConfig::setPort(int p)
{
	port = p;
	return *this;
}

ServerConfig &ServerConfig::addServerName(const std::string &name)
{
	serverNames.push_back(name);
	return *this;
}

ServerConfig &ServerConfig::setRoot(const std::string &r)
{
	root = r;
	return *this;
}

ServerConfig &ServerConfig::addIndex(const std::string &indexFile)
{
	index.push_back(indexFile);
	return *this;
}

ServerConfig &ServerConfig::setClientMaxBodySize(size_t size)
{
	clientMaxBodySize = size;
	return *this;
}

ServerConfig &ServerConfig::addErrorPage(int statusCode, const std::string &path)
{
	errorPages[statusCode] = path;
	return *this;
}

ServerConfig &ServerConfig::addLocation(const LocationConfig &location)
{
	locations.push_back(location);
	return *this;
}

// Getters
std::string ServerConfig::getHost() const
{
	return host;
}

int ServerConfig::getPort() const
{
	return port;
}

const std::vector<std::string> &ServerConfig::getServerNames() const
{
	return serverNames;
}

std::string ServerConfig::getRoot() const
{
	return root;
}

const std::vector<std::string> &ServerConfig::getIndex() const
{
	return index;
}

size_t ServerConfig::getClientMaxBodySize() const
{
	return clientMaxBodySize;
}

std::string ServerConfig::getErrorPage(int statusCode) const
{
	std::map<int, std::string>::const_iterator it = errorPages.find(statusCode);
	if (it != errorPages.end())
		return it->second;
	return "";
}

const std::vector<LocationConfig> &ServerConfig::getLocations() const
{
	return locations;
}

// Utility
void ServerConfig::clear()
{
	host = DEFAULT_HOST;
	port = DEFAULT_PORT;
	serverNames.clear();
	serverNames.push_back("localhost");
	root = DEFAULT_ROOT;
	index.clear();
	index.push_back(DEFAULT_INDEX);
	index.push_back("index.htm");
	clientMaxBodySize = MAX_BODY_SIZE;
	errorPages.clear();
	locations.clear();
}

bool ServerConfig::isValid() const
{
	// Port must be in valid range
	if (port < 1 || port > 65535)
		return false;
	
	// Host must not be empty
	if (host.empty())
		return false;
	
	// Root must not be empty
	if (root.empty())
		return false;
	
	return true;
}
