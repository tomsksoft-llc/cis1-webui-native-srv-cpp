/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "request_context.h"

#include "tpl_helpers/overloaded.h"

std::optional<std::string> request_context::username(const client_info_holder& client_info)
{
    return std::visit(
            meta::overloaded{
                    [](const request_context::user_info& ctx) -> std::optional<std::string>
                    { return ctx.username; },
                    [](const request_context::guest_info&) -> std::optional<std::string>
                    { return std::nullopt; }
            },
            client_info
    );
}

std::string request_context::user_or_guest_name(const client_info_holder& client_info)
{
    return std::visit(
            meta::overloaded{
                    [](const request_context::user_info& ctx)
                    { return ctx.username; },
                    [](const request_context::guest_info&)
                    { return std::string{request_context::guest_info::guestname};}
            },
            client_info
    );
}

std::string request_context::username_or_empty(const client_info_holder& client_info)
{
    const auto opt_username = username(client_info);
    if(opt_username)
    {
        return opt_username.value();
    }
    return std::string{};
}

std::string request_context::active_token_or_empty(const client_info_holder& client_info)
{
    return std::visit(
            meta::overloaded{
                    [](const request_context::user_info& ctx)
                    { return ctx.active_token; },
                    [](const request_context::guest_info&)
                    { return std::string{}; }
            },
            client_info
    );
}

std::string request_context::api_access_key_or_empty(const client_info_holder& client_info)
{
    return std::visit(
            meta::overloaded{
                    [](const request_context::user_info& ctx)
                    { return ctx.api_access_key; },
                    [](const request_context::guest_info&)
                    { return std::string{}; }
            },
            client_info
    );
}
