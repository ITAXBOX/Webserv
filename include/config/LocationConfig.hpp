#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

// LocationConfig: Configuration for a location block
// Represents a location directive in the config file
class LocationConfig
{
private:
	std::string path;						 // Location path (e.g., "/", "/api", "/images")
	std::string root;						 // Root directory for this location (overrides server root)
	std::vector<std::string> index;			 // Index files for this location
	std::set<std::string> allowedMethods;	 // Allowed HTTP methods (GET, POST, PUT, DELETE, HEAD)
	bool autoindex;							 // Enable directory listing
	size_t clientMaxBodySize;				 // Maximum request body size in bytes
    std::map<std::string, std::string> cgiHandlers; // Map extension -> interpreter path
	std::string redirect;					 // Redirect URL (if any)
	int redirectCode;						 // Redirect status code (301, 302, etc.)

public:
	LocationConfig();
	LocationConfig(const std::string &path); // Convenience constructor
	~LocationConfig();

	// Setters (Builder pattern)
	LocationConfig &setPath(const std::string &path);
	LocationConfig &setRoot(const std::string &root);
	LocationConfig &addIndex(const std::string &indexFile);
	LocationConfig &addAllowedMethod(const std::string &method);
	LocationConfig &setAutoindex(bool enabled);
	LocationConfig &setClientMaxBodySize(size_t size);
    LocationConfig &addCgiHandler(const std::string &extension, const std::string &interpreterPath);
	LocationConfig &setRedirect(const std::string &url, int code = 301);

	// Getters
	std::string getPath() const;
	std::string getRoot() const;
	const std::vector<std::string> &getIndex() const;
	const std::set<std::string> &getAllowedMethods() const;
	bool isMethodAllowed(const std::string &method) const;
	bool getAutoindex() const;
	size_t getClientMaxBodySize() const;
    std::string getCgiPath(const std::string &extension) const;
    const std::map<std::string, std::string> &getCgiHandlers() const;
	std::string getRedirect() const;
	int getRedirectCode() const;
	bool hasRedirect() const;

	// Utility
	void clear();
    void clearAllowedMethods();
	bool isValid() const;
};

#endif
