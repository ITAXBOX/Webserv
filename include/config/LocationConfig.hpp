#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <set>

// LocationConfig: Configuration for a location block
// Represents a location directive in the config file
class LocationConfig
{
private:
	std::string path;						 // Location path (e.g., "/", "/api", "/images")
	std::string root;						 // Root directory for this location (overrides server root)
	std::vector<std::string> index;			 // Index files for this location
	std::set<std::string> allowedMethods;	 // Allowed HTTP methods (GET, POST, DELETE)
	bool autoindex;							 // Enable directory listing
	std::string uploadPath;					 // Directory for file uploads
	std::string cgiExtension;				 // CGI file extension (e.g., ".py", ".php")
	std::string cgiPath;					 // Path to CGI interpreter
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
	LocationConfig &setUploadPath(const std::string &path);
	LocationConfig &setCgiExtension(const std::string &extension);
	LocationConfig &setCgiPath(const std::string &path);
	LocationConfig &setRedirect(const std::string &url, int code = 301);

	// Getters
	std::string getPath() const;
	std::string getRoot() const;
	const std::vector<std::string> &getIndex() const;
	const std::set<std::string> &getAllowedMethods() const;
	bool isMethodAllowed(const std::string &method) const;
	bool getAutoindex() const;
	std::string getUploadPath() const;
	std::string getCgiExtension() const;
	std::string getCgiPath() const;
	std::string getRedirect() const;
	int getRedirectCode() const;
	bool hasRedirect() const;

	// Utility
	void clear();
	bool isValid() const;
};

#endif
