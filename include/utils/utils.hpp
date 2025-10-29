#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include "http/HttpRequest.hpp"

void printWebservStartup();

// String utilities
std::string trim(const std::string &str);
size_t findCRLF(const std::string &str, size_t start = 0);

// HTTP utilities
HttpMethod stringToHttpMethod(const std::string &method);

#endif