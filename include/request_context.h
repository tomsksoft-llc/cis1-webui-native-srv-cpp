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

#include <boost/beast/http/status.hpp>

struct request_context
{
    struct user_info
    {
        std::string username;
        std::string active_token;
        std::string api_access_key;
    };

    struct guest_info
    {
        const std::string guestname = "guest";
    };

    using cln_info_holder =  std::variant<guest_info, user_info>;

    // authorized user's info
    // not null if the client has been authorized else nullopt
    cln_info_holder cln_info = guest_info{};
    uint64_t session_id;
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::any> other;
    boost::beast::http::status res_status = boost::beast::http::status::unknown;
    std::string error;
};
