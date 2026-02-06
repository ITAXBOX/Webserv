#include "config/LocationConfig.hpp"

LocationConfig::LocationConfig()
	: path("/"),
	  root(""),
	  autoindex(false),
	  clientMaxBodySize(0), // 0 means not set (inherit from server)
	  redirect(""),
	  redirectCode(0)
{
	// No default methods - will be set explicitly
}

LocationConfig::LocationConfig(const std::string &p)
	: path(p),
	  root(""),
	  autoindex(false),
	  clientMaxBodySize(0), // 0 means not set (inherit from server)
	  redirect(""),
	  redirectCode(0)
{
	// No default methods - will be set explicitly
}

LocationConfig::~LocationConfig()
{
}

// Setters (Builder pattern)
LocationConfig &LocationConfig::setPath(const std::string &p)
{
	path = p;
	return *this;
}

LocationConfig &LocationConfig::setRoot(const std::string &r)
{
	root = r;
	return *this;
}

LocationConfig &LocationConfig::addIndex(const std::string &indexFile)
{
	index.push_back(indexFile);
	return *this;
}

void LocationConfig::clearAllowedMethods()
{
    allowedMethods.clear();
}

LocationConfig &LocationConfig::addAllowedMethod(const std::string &method)
{
	allowedMethods.insert(method);
	return *this;
}

LocationConfig &LocationConfig::setAutoindex(bool enabled)
{
	autoindex = enabled;
	return *this;
}

LocationConfig &LocationConfig::setClientMaxBodySize(size_t size)
{
	clientMaxBodySize = size;
	return *this;
}

LocationConfig &LocationConfig::addCgiHandler(const std::string &extension, const std::string &interpreterPath)
{
	cgiHandlers[extension] = interpreterPath;
	return *this;
}

LocationConfig &LocationConfig::setRedirect(const std::string &url, int code)
{
	redirect = url;
	redirectCode = code;
	return *this;
}

// Getters
std::string LocationConfig::getPath() const
{
	return path;
}

std::string LocationConfig::getRoot() const
{
	return root;
}

const std::vector<std::string> &LocationConfig::getIndex() const
{
	return index;
}

const std::set<std::string> &LocationConfig::getAllowedMethods() const
{
	return allowedMethods;
}

bool LocationConfig::isMethodAllowed(const std::string &method) const
{
	return allowedMethods.find(method) != allowedMethods.end();
}

bool LocationConfig::getAutoindex() const
{
	return autoindex;
}

size_t LocationConfig::getClientMaxBodySize() const
{
	return clientMaxBodySize;
}

std::string LocationConfig::getCgiPath(const std::string &extension) const
{
	std::map<std::string, std::string>::const_iterator it = cgiHandlers.find(extension);
    if (it != cgiHandlers.end())
        return it->second;
    return "";
}

const std::map<std::string, std::string> &LocationConfig::getCgiHandlers() const
{
    return cgiHandlers;
}

std::string LocationConfig::getRedirect() const
{
	return redirect;
}

int LocationConfig::getRedirectCode() const
{
	return redirectCode;
}

bool LocationConfig::hasRedirect() const
{
	return !redirect.empty();
}

// Utility
void LocationConfig::clear()
{
	path = "/";
	root.clear();
	index.clear();
	allowedMethods.clear();
	allowedMethods.insert("GET");
	allowedMethods.insert("HEAD");
	autoindex = false;
	cgiHandlers.clear();
	redirect.clear();
	redirectCode = 0;
}

bool LocationConfig::isValid() const
{
	// Path must start with /
	if (path.empty() || path[0] != '/')
		return false;
	
	// If redirect is set, redirect code must be valid
	if (!redirect.empty() && (redirectCode < 300 || redirectCode > 399))
		return false;
	
	return true;
}
