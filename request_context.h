#pragma once

#include <map>
#include <string>
#include <any>

class request_context
{
public:
    std::string username;
    std::map<std::string, std::string> cookies;
    std::string active_token;
    std::map<std::string, std::any> other;
};
