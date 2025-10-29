#include "config/ServerConfig.hpp"

ServerConfig::ServerConfig()
	: host("0.0.0.0"),				// Listen on all interfaces by default
	  port(8080),					// Default port
	  root("./www"),				// Default web root
	  clientMaxBodySize(1048576)	// 1MB default (1024 * 1024)
{
	// Default index files
	index.push_back("index.html");
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
	host = "0.0.0.0";
	port = 8080;
	serverNames.clear();
	serverNames.push_back("localhost");
	root = "./www";
	index.clear();
	index.push_back("index.html");
	index.push_back("index.htm");
	clientMaxBodySize = 1048576;
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
