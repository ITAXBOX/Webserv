#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <map>
#include "config/LocationConfig.hpp"

// Forward declaration
class LocationConfig;

// ServerConfig: Configuration for a virtual server
// Represents a server block in the config file
class ServerConfig
{
private:
	std::string host;						   // IP address to bind to (e.g., "0.0.0.0", "127.0.0.1")
	int port;								   // Port to listen on (e.g., 8080)
	std::vector<std::string> serverNames;	   // Server names (e.g., "example.com", "www.example.com")
	std::string root;						   // Root directory for serving files
	std::vector<std::string> index;			   // Default index files (e.g., ["index.html", "index.htm"])
	size_t clientMaxBodySize;				   // Maximum request body size in bytes
	std::map<int, std::string> errorPages;	   // Custom error pages (status code -> file path)
	std::vector<LocationConfig> locations;	   // Location blocks for this server

public:
	ServerConfig();
	~ServerConfig();

	// Setters (Builder pattern)
	ServerConfig &setHost(const std::string &host);
	ServerConfig &setPort(int port);
	ServerConfig &addServerName(const std::string &name);
	ServerConfig &setRoot(const std::string &root);
	ServerConfig &addIndex(const std::string &indexFile);
	ServerConfig &setClientMaxBodySize(size_t size);
	ServerConfig &addErrorPage(int statusCode, const std::string &path);
	ServerConfig &addLocation(const LocationConfig &location);

	// Getters
	std::string getHost() const;
	int getPort() const;
	const std::vector<std::string> &getServerNames() const;
	std::string getRoot() const;
	const std::vector<std::string> &getIndex() const;
	size_t getClientMaxBodySize() const;
	std::string getErrorPage(int statusCode) const;
	const std::vector<LocationConfig> &getLocations() const;

	// Utility
	void clear();
	bool isValid() const; // Check if configuration is valid
};

#endif
