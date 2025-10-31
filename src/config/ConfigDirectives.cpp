#include "config/ConfigDirectives.hpp"
#include "utils/Logger.hpp"
#include <sstream>
#include <cstdlib>

// ============================================================================
// Utility Functions
// ============================================================================

Token ConfigDirectives::peek(const std::vector<Token> &tokens, size_t pos)
{
	if (pos < tokens.size())
		return tokens[pos];
	return Token(TOKEN_EOF, "", -1);
}

Token ConfigDirectives::advance(std::vector<Token> &tokens, size_t &pos)
{
	if (pos < tokens.size())
		return tokens[pos++];
	return Token(TOKEN_EOF, "", -1);
}

void ConfigDirectives::setError(std::string &error, const std::string &msg, int line)
{
	std::ostringstream os;
	os << "Line " << line << ": " << msg;
	error = os.str();
	Logger::error("Config parse error: " + error);
}

bool ConfigDirectives::expectSemicolon(std::vector<Token> &tokens, size_t &pos, std::string &error)
{
	Token token = peek(tokens, pos);
	if (token.type != TOKEN_SEMICOLON)
	{
		setError(error, "Expected ';'", token.line);
		return false;
	}
	advance(tokens, pos);
	return true;
}

// ============================================================================
// Server Directive Parsers
// ============================================================================

bool ConfigDirectives::parseListen(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error)
{
	Token directive = advance(tokens, pos); // Consume 'listen'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected port number after 'listen'", value.line);
		return false;
	}
	
	int port = std::atoi(value.value.c_str());
	if (port <= 0 || port > 65535)
	{
		setError(error, "Invalid port number: " + value.value, value.line);
		return false;
	}
	
	server.setPort(port);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseServerName(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error)
{
	advance(tokens, pos); // Consume 'server_name'
	
	// Can have multiple server names
	while (peek(tokens, pos).type == TOKEN_WORD)
	{
		Token name = advance(tokens, pos);
		server.addServerName(name.value);
	}
	
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseRoot(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error)
{
	advance(tokens, pos); // Consume 'root'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected path after 'root'", value.line);
		return false;
	}
	
	server.setRoot(value.value);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseIndex(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error)
{
	advance(tokens, pos); // Consume 'index'
	
	// Can have multiple index files
	while (peek(tokens, pos).type == TOKEN_WORD)
	{
		Token indexFile = advance(tokens, pos);
		server.addIndex(indexFile.value);
	}
	
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseClientMaxBodySize(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error)
{
	advance(tokens, pos); // Consume 'client_max_body_size'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected size after 'client_max_body_size'", value.line);
		return false;
	}
	
	size_t size = std::atol(value.value.c_str());
	server.setClientMaxBodySize(size);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseErrorPage(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error)
{
	Token directive = advance(tokens, pos); // Consume 'error_page'
	Token code = advance(tokens, pos);
	Token path = advance(tokens, pos);
	
	if (code.type != TOKEN_WORD || path.type != TOKEN_WORD)
	{
		setError(error, "Expected 'error_page <code> <path>;'", directive.line);
		return false;
	}
	
	int statusCode = std::atoi(code.value.c_str());
	server.addErrorPage(statusCode, path.value);
	return expectSemicolon(tokens, pos, error);
}

// ============================================================================
// Location Directive Parsers
// ============================================================================

bool ConfigDirectives::parseAllowedMethods(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'allowed_methods'
	
	// Can have multiple methods
	while (peek(tokens, pos).type == TOKEN_WORD)
	{
		Token method = advance(tokens, pos);
		location.addAllowedMethod(method.value);
	}
	
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseLocationRoot(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'root'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected path after 'root'", value.line);
		return false;
	}
	
	location.setRoot(value.value);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseLocationIndex(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'index'
	
	// Can have multiple index files
	while (peek(tokens, pos).type == TOKEN_WORD)
	{
		Token indexFile = advance(tokens, pos);
		location.addIndex(indexFile.value);
	}
	
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseAutoindex(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'autoindex'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected 'on' or 'off' after 'autoindex'", value.line);
		return false;
	}
	
	bool enabled = (value.value == "on");
	location.setAutoindex(enabled);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseUploadPath(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'upload_path'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected path after 'upload_path'", value.line);
		return false;
	}
	
	location.setUploadPath(value.value);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseCgiExtension(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'cgi_extension'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected extension after 'cgi_extension'", value.line);
		return false;
	}
	
	location.setCgiExtension(value.value);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseCgiPath(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'cgi_path'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected path after 'cgi_path'", value.line);
		return false;
	}
	
	location.setCgiPath(value.value);
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseReturn(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	Token directive = advance(tokens, pos); // Consume 'return'
	Token code = advance(tokens, pos);
	Token url = advance(tokens, pos);
	
	if (code.type != TOKEN_WORD || url.type != TOKEN_WORD)
	{
		setError(error, "Expected 'return <code> <url>;'", directive.line);
		return false;
	}
	
	int redirectCode = std::atoi(code.value.c_str());
	location.setRedirect(url.value, redirectCode);
	return expectSemicolon(tokens, pos, error);
}
