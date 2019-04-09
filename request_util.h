#pragma once

#include <string>
#include <map>

std::map<std::string, std::string> parse_request_query(const std::string& body);

std::map<std::string, std::string> parse_cookies(const std::string& cookies);
