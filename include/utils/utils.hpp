#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <sstream>
#include "http/HttpRequest.hpp"

void printWebservStartup();

// String utilities
std::string trim(const std::string &str);
size_t findCRLF(const std::string &str, size_t start = 0);

// Template for numeric to string conversion
template <typename T>
std::string toString(T n)
{
	std::ostringstream os;
	os << n;
	return os.str();
}

// Config parsing utilities
void skipWhitespace(const std::string &input, size_t &pos, int &line);
void skipComment(const std::string &input, size_t &pos);
bool isWordChar(char c);

// HTTP utilities
HttpMethod stringToHttpMethod(const std::string &method);

// APP utilities
std::string buildFilePath(const std::string &uri, const std::string &rootDir, const std::string &defaultIndex);
#endif