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
        static constexpr auto guestname = "guest";
    };

    using client_info_holder = std::variant<guest_info, user_info>;

    static std::optional<std::string> username(const client_info_holder& client_info);

    static std::string user_or_guest_name(const client_info_holder& client_info);

    static std::string username_or_empty(const client_info_holder& client_info);

    static std::string active_token_or_empty(const client_info_holder& client_info);

    static std::string api_access_key_or_empty(const client_info_holder& client_info);

    static bool authorized(const client_info_holder& client_info);

    client_info_holder client_info = guest_info{};
    uint64_t session_id;
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::any> other;
    boost::beast::http::status res_status = boost::beast::http::status::unknown;
    std::string error;
};
