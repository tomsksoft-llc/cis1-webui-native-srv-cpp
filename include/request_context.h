/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <map>
#include <string>
#include <any>
#include <variant>
#include <vector>

#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

struct request_context
{
    struct user_info
    {
        std::string email;
        std::string active_token;
        std::string api_access_key;
    };

    std::optional<user_info> client_info = std::nullopt;
    uint64_t session_id;
    std::map<std::string, std::string> cookies;
    boost::beast::http::status res_status = boost::beast::http::status::unknown;
    std::vector<boost::beast::http::verb> allowed_verbs;
    std::string error;
    std::pair<std::string, unsigned short> remote_addr;
};
