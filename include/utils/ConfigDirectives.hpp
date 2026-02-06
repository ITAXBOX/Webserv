#ifndef CONFIGDIRECTIVES_HPP
#define CONFIGDIRECTIVES_HPP

#include "config/Token.hpp"
#include "config/ServerConfig.hpp"
#include "config/LocationConfig.hpp"
#include <string>
#include <vector>

// Helper class for parsing config directives
// Separates parsing logic from the main ConfigParser
class ConfigDirectives
{
public:
	// Server directive parsers
	static bool parseListen(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error);
	static bool parseHost(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error);
	static bool parseServerName(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error);
	static bool parseRoot(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error);
	static bool parseIndex(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error);
	static bool parseClientMaxBodySize(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error);
	static bool parseErrorPage(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error);

	// Location directive parsers
	static bool parseAllowedMethods(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);
	static bool parseLocationRoot(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);
	static bool parseLocationIndex(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);
	static bool parseAutoindex(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);
	static bool parseClientMaxBodySize(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);
	static bool parseUploadStore(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);
	static bool parseCgiAssign(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);
	static bool parseReturn(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error);

	// Utility functions
	static bool expectSemicolon(std::vector<Token> &tokens, size_t &pos, std::string &error);
	static Token peek(const std::vector<Token> &tokens, size_t pos);
	static Token advance(std::vector<Token> &tokens, size_t &pos);
	static void setError(std::string &error, const std::string &msg, int line);
};

#endif
