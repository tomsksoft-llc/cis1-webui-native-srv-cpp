#pragma once

#include <map>
#include <string>
#include <any>

#include <boost/beast/http/status.hpp>

class request_context
{
public:
    std::string username;
    std::map<std::string, std::string> cookies;
    std::string active_token;
    std::map<std::string, std::any> other;
    boost::beast::http::status res_status = boost::beast::http::status::unknown;
    std::string error;
};
