#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "config/Token.hpp"
#include "config/ServerConfig.hpp"
#include <vector>
#include <string>

// ConfigParser: Parses tokens into ServerConfig objects
// Takes output from Tokenizer and builds configuration structure
class ConfigParser
{
private:
	std::vector<Token> _tokens;			// All tokens from tokenizer
	size_t _pos;						// Current position in tokens
	std::string _error;					// Error message if parsing fails
	std::vector<ServerConfig> _servers;	// Parsed server configurations

	// Helper methods
	Token peek() const;					// Look at current token without consuming
	Token advance();					// Consume and return current token
	bool expect(TokenType type);		// Check if current token matches type
	bool expectWord(const std::string &word); // Check if current token is specific word
	
	// Parsing methods
	bool parseServer();					// Parse a server block
	bool parseServerDirective(ServerConfig &server);
	bool parseLocation(ServerConfig &server);
	bool parseLocationDirective(LocationConfig &location);
	
	// Utility
	void setError(const std::string &msg, int line);
	bool isServerDirective(const std::string &word) const;
	bool isLocationDirective(const std::string &word) const;

public:
	ConfigParser();
	~ConfigParser();

	// Parse tokens into server configurations
	bool parse(const std::vector<Token> &tokens);

	// Get parsed servers
	const std::vector<ServerConfig>& getServers() const;

	// Get error message if parsing failed
	std::string getError() const;
};

#endif
