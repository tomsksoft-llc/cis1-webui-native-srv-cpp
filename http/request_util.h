#pragma once

#include <string>
#include <map>

namespace http
{

unsigned char from_hex(unsigned char ch);

unsigned char to_hex(unsigned char ch);

std::string unescape_uri(const std::string& escaped_uri);

std::string escape_uri(const std::string& unescaped_uri);

std::map<std::string, std::string> parse_request_query(const std::string& body);

std::map<std::string, std::string> parse_cookies(const std::string& cookies);

} // namespace http
