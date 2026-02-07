#include "config/ConfigParser.hpp"

ConfigParser::ConfigParser() : _pos(0), _error("")
{
}

ConfigParser::~ConfigParser()
{
}

// ============================================================================
// Helper Methods
// ============================================================================

Token ConfigParser::peek() const
{
	return ConfigDirectives::peek(_tokens, _pos);
}

Token ConfigParser::advance()
{
	return ConfigDirectives::advance(_tokens, _pos);
}

bool ConfigParser::expect(TokenType type)
{
	Token token = peek();
	if (token.type != type)
	{
		std::ostringstream os;
		os << "Expected token type " << type << " but got " << token.type;
		setError(os.str(), token.line);
		return false;
	}
	return true;
}

void ConfigParser::setError(const std::string &msg, int line)
{
	ConfigDirectives::setError(_error, msg, line);
}

// Check if word is a server directive
bool ConfigParser::isServerDirective(const std::string &word) const
{
	return word == "listen" || word == "host" || word == "server_name" || word == "root" ||
		   word == "index" || word == "client_max_body_size" ||
		   word == "error_page" || word == "location";
}

// Check if word is a location directive
bool ConfigParser::isLocationDirective(const std::string &word) const
{
	return word == "root" || word == "index" || word == "allowed_methods" ||
		   word == "autoindex" || word == "client_max_body_size" ||
		   word == "upload_store" || word == "cgi_assign" || word == "return";
}

// ============================================================================
// Parsing Methods
// ============================================================================

// Parse: server { ... }
bool ConfigParser::parseServer()
{
	Token token = advance(); // Consume 'server'

	if (!expect(TOKEN_OPEN_BRACE))
		return false;
	advance(); // Consume '{'

	ServerConfig server;

	// Parse directives inside server block
	while (true)
	{
		token = peek();

		if (token.type == TOKEN_CLOSE_BRACE)
		{
			advance(); // Consume '}'
			break;
		}

		if (token.type == TOKEN_EOF)
		{
			setError("Unexpected EOF while parsing server block", token.line);
			return false;
		}

		if (token.type == TOKEN_WORD)
		{
			if (token.value == "location")
			{
				if (!parseLocation(server))
					return false;
			}
			else if (isServerDirective(token.value))
			{
				if (!parseServerDirective(server))
					return false;
			}
			else
			{
				setError("Unknown server directive: " + token.value, token.line);
				return false;
			}
		}
		else
		{
			setError("Expected directive in server block", token.line);
			return false;
		}
	}

	_servers.push_back(server);
	return true;
}

// Parse server directives (listen, server_name, root, etc.)
bool ConfigParser::parseServerDirective(ServerConfig &server)
{
	Token directive = peek();

	if (directive.value == "listen")
		return ConfigDirectives::parseListen(_tokens, _pos, server, _error);
	else if (directive.value == "host")
		return ConfigDirectives::parseHost(_tokens, _pos, server, _error);
	else if (directive.value == "server_name")
		return ConfigDirectives::parseServerName(_tokens, _pos, server, _error);
	else if (directive.value == "root")
		return ConfigDirectives::parseRoot(_tokens, _pos, server, _error);
	else if (directive.value == "index")
		return ConfigDirectives::parseIndex(_tokens, _pos, server, _error);
	else if (directive.value == "client_max_body_size")
		return ConfigDirectives::parseClientMaxBodySize(_tokens, _pos, server, _error);
	else if (directive.value == "error_page")
		return ConfigDirectives::parseErrorPage(_tokens, _pos, server, _error);

	return true;
}

// Parse: location /path { ... }
bool ConfigParser::parseLocation(ServerConfig &server)
{
	Token token = advance(); // Consume 'location'

	Token path = advance();
	if (path.type != TOKEN_WORD)
	{
		setError("Expected path after 'location'", path.line);
		return false;
	}

	if (!expect(TOKEN_OPEN_BRACE))
		return false;
	advance(); // Consume '{'

	LocationConfig location(path.value);

	// Parse directives inside location block
	while (true)
	{
		token = peek();

		if (token.type == TOKEN_CLOSE_BRACE)
		{
			advance(); // Consume '}'
			break;
		}

		if (token.type == TOKEN_EOF)
		{
			setError("Unexpected EOF while parsing location block", token.line);
			return false;
		}

		if (token.type == TOKEN_WORD)
		{
			if (isLocationDirective(token.value))
			{
				if (!parseLocationDirective(location))
					return false;
			}
			else
			{
				setError("Unknown location directive: " + token.value, token.line);
				return false;
			}
		}
		else
		{
			setError("Expected directive in location block", token.line);
			return false;
		}
	}

	server.addLocation(location);
	return true;
}

// Parse location directives (allowed_methods, root, etc.)
bool ConfigParser::parseLocationDirective(LocationConfig &location)
{
	Token directive = peek();

	if (directive.value == "allowed_methods")
		return ConfigDirectives::parseAllowedMethods(_tokens, _pos, location, _error);
	else if (directive.value == "root")
		return ConfigDirectives::parseLocationRoot(_tokens, _pos, location, _error);
	else if (directive.value == "index")
		return ConfigDirectives::parseLocationIndex(_tokens, _pos, location, _error);
	else if (directive.value == "autoindex")
		return ConfigDirectives::parseAutoindex(_tokens, _pos, location, _error);
	else if (directive.value == "client_max_body_size")
		return ConfigDirectives::parseClientMaxBodySize(_tokens, _pos, location, _error);
	else if (directive.value == "upload_store")
		return ConfigDirectives::parseUploadStore(_tokens, _pos, location, _error);
	else if (directive.value == "cgi_assign")
		return ConfigDirectives::parseCgiAssign(_tokens, _pos, location, _error);
	else if (directive.value == "return")
		return ConfigDirectives::parseReturn(_tokens, _pos, location, _error);

	return true;
}

// ============================================================================
// Public API
// ============================================================================

// Parse tokens into server configurations
bool ConfigParser::parse(const std::vector<Token> &tokens)
{
	_tokens = tokens;
	_pos = 0;
	_error = "";
	_servers.clear();

	// Parse all server blocks
	while (peek().type != TOKEN_EOF)
	{
		Token token = peek();

		if (token.type == TOKEN_WORD && token.value == "server")
		{
			if (!parseServer())
				return false;
		}
		else
		{
			setError("Expected 'server' directive at top level", token.line);
			return false;
		}
	}

	if (_servers.empty())
	{
		_error = "No server blocks found in configuration";
		Logger::error("Config parse error: " + _error);
		return false;
	}

	return true;
}

// Get parsed servers
const std::vector<ServerConfig> &ConfigParser::getServers() const
{
	return _servers;
}

// Get error message
std::string ConfigParser::getError() const
{
	return _error;
}
