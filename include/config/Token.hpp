#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

// Token types for config file parsing
enum TokenType
{
	TOKEN_WORD,		   // Identifiers, keywords, strings (server, listen, 8080, etc.)
	TOKEN_OPEN_BRACE,  // {
	TOKEN_CLOSE_BRACE, // }
	TOKEN_SEMICOLON,   // ;
	TOKEN_EOF,		   // End of file
	TOKEN_ERROR		   // Lexical error
};

struct Token
{
	TokenType type;
	std::string value; // The actual text (for TOKEN_WORD)
	int line;		   // Line number in config file (for error reporting)

	Token() : type(TOKEN_EOF), value(""), line(0)
	{
	}
	Token(TokenType t, const std::string &val, int ln)
		: type(t), value(val), line(ln)
	{
	}
};

#endif
