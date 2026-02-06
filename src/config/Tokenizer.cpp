#include "config/Tokenizer.hpp"

Tokenizer::Tokenizer() : _input(""), _pos(0), _line(1)
{
}

Tokenizer::~Tokenizer()
{
}

// Read a word token (identifier, keyword, number, path, etc.)
Token Tokenizer::readWord()
{
	size_t start = _pos;
	while (_pos < _input.length() && isWordChar(_input[_pos]))
		_pos++;

	std::string value = _input.substr(start, _pos - start);
	return Token(TOKEN_WORD, value, _line);
}

// Read the next token from input
Token Tokenizer::readToken()
{
	while (true)
	{
		skipWhitespace(_input, _pos, _line);

		// Check for comment
		if (_pos < _input.length() && _input[_pos] == '#')
		{
			skipComment(_input, _pos);
			continue; // Loop back to skip more whitespace
		}

		break; // No more whitespace or comments
	}

	// EOF
	if (_pos >= _input.length())
		return Token(TOKEN_EOF, "", _line);

	char c = _input[_pos];

	// Single-character tokens
	if (c == '{')
	{
		_pos++;
		return Token(TOKEN_OPEN_BRACE, "{", _line);
	}
	if (c == '}')
	{
		_pos++;
		return Token(TOKEN_CLOSE_BRACE, "}", _line);
	}
	if (c == ';')
	{
		_pos++;
		return Token(TOKEN_SEMICOLON, ";", _line);
	}

	// Word token (identifier, keyword, value)
	if (isWordChar(c))
		return readWord();

	// Unknown character - error
	_pos++;
	return Token(TOKEN_ERROR, std::string(1, c), _line);
}

// Tokenize the entire input
bool Tokenizer::tokenize(const std::string &input)
{
	_input = input;
	_pos = 0;
	_line = 1;
	_tokens.clear();

	while (true)
	{
		Token token = readToken();
		_tokens.push_back(token);

		if (token.type == TOKEN_EOF)
			break;

		if (token.type == TOKEN_ERROR)
			return false;
	}

	return true;
}

// Get all tokens
const std::vector<Token> &Tokenizer::getTokens() const
{
	return _tokens;
}

// Get error message
std::string Tokenizer::getError() const
{
	if (_tokens.empty())
		return "No tokens generated";

	// Find first error token
	for (size_t i = 0; i < _tokens.size(); i++)
	{
		if (_tokens[i].type == TOKEN_ERROR)
		{
			std::ostringstream os;
			os << "Unexpected character '" << _tokens[i].value
			   << "' at line " << _tokens[i].line;
			return os.str();
		}
	}

	return "Unknown tokenization error";
}
