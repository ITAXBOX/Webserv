#include "utils/ConfigDirectives.hpp"
#include "utils/Logger.hpp"
#include <sstream>
#include <cstdlib>
#include <cctype>

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
		setError(error, "Expected port number or host:port after 'listen'", value.line);
		return false;
	}
	
	std::string val = value.value;
	size_t colonPos = val.find(':');

	if (colonPos != std::string::npos)
	{
		// Format: host:port
		std::string host = val.substr(0, colonPos);
		std::string portStr = val.substr(colonPos + 1);
		
		server.setHost(host);
		
		int port = std::atoi(portStr.c_str());
		if (port <= 0 || port > 65535)
		{
			setError(error, "Invalid port number: " + portStr, value.line);
			return false;
		}
		server.setPort(port);
	}
	else
	{
		// Format: port OR host
		bool isPort = true;
		for (size_t i = 0; i < val.length(); i++)
		{
			if (!std::isdigit(static_cast<unsigned char>(val[i])))
			{
				isPort = false;
				break;
			}
		}

		if (isPort)
		{
			int port = std::atoi(val.c_str());
			if (port <= 0 || port > 65535)
			{
				setError(error, "Invalid port number: " + val, value.line);
				return false;
			}
			server.setPort(port);
		}
		else
		{
			// Treat as host only (e.g. "localhost" or "127.0.0.1")
			server.setHost(val);
		}
	}
	
	return expectSemicolon(tokens, pos, error);
}

bool ConfigDirectives::parseHost(std::vector<Token> &tokens, size_t &pos, ServerConfig &server, std::string &error)
{
	advance(tokens, pos); // Consume 'host'
	Token value = advance(tokens, pos);
	
	if (value.type != TOKEN_WORD)
	{
		setError(error, "Expected host address after 'host'", value.line);
		return false;
	}
	
	server.setHost(value.value);
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
	
    // Clear default methods (GET, HEAD) so we only allow what is explicitly specified
    location.clearAllowedMethods();

	// Valid HTTP/1.1 methods that are implemented in this server
	std::set<std::string> validMethods;
	validMethods.insert("GET");
	validMethods.insert("POST");
	validMethods.insert("PUT");
	validMethods.insert("DELETE");
	validMethods.insert("HEAD");

	// Can have multiple methods
	bool hasAtLeastOne = false;
	while (peek(tokens, pos).type == TOKEN_WORD)
	{
		Token method = advance(tokens, pos);
		
		// Validate that the method is one of the implemented ones
		if (validMethods.find(method.value) == validMethods.end())
		{
			setError(error, "Invalid HTTP method '" + method.value + "'. Allowed methods: GET, POST, PUT, DELETE, HEAD", method.line);
			return false;
		}
		
		location.addAllowedMethod(method.value);
		hasAtLeastOne = true;
	}
	
	// Ensure at least one method was specified
	if (!hasAtLeastOne)
	{
		setError(error, "Expected at least one HTTP method after 'allowed_methods'", peek(tokens, pos).line);
		return false;
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

bool ConfigDirectives::parseCgiAssign(std::vector<Token> &tokens, size_t &pos, LocationConfig &location, std::string &error)
{
	advance(tokens, pos); // Consume 'cgi_assign'
	Token extension = advance(tokens, pos);
    Token path = advance(tokens, pos);
	
	if (extension.type != TOKEN_WORD || path.type != TOKEN_WORD)
	{
		setError(error, "Expected extension and path after 'cgi_assign'", extension.line);
		return false;
	}
	
	location.addCgiHandler(extension.value, path.value);
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
