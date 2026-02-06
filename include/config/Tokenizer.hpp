#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include "config/Token.hpp"
#include "utils/utils.hpp"
#include <sstream>
#include <string>
#include <vector>

// Tokenizer (Lexer) for nginx-style config files
// Breaks config file into tokens: words, braces, semicolons
// Handles comments (#) and tracks line numbers
class Tokenizer
{
private:
	std::string _input;			// The entire config file content
	size_t _pos;				// Current position in input
	int _line;					// Current line number (for error reporting)
	std::vector<Token> _tokens; // All tokens generated

	Token readWord();
	Token readToken();

public:
	Tokenizer();
	~Tokenizer();

	// Tokenize the entire input string
	bool tokenize(const std::string &input);

	// Get all tokens
	const std::vector<Token> &getTokens() const;

	// Get error message if tokenization failed
	std::string getError() const;
};

#endif
