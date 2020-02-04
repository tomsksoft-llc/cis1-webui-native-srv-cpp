/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_api_key.h"

#include <tpl_helpers/overloaded.h>

#include "websocket/dto/user_api_key_get_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void get_api_key(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_get& req,
        cis1::proto_utils::transaction tr)
{
    const auto opt_ctx_username = std::visit(
            meta::overloaded{
                    [](const request_context::user_info& ctx) -> std::optional<std::string>
                    { return ctx.username; },
                    [](const request_context::guest_info& ctx) -> std::optional<std::string>
                    { return std::nullopt; }
            },
            ctx.cln_info
    );

    if(!opt_ctx_username)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    const auto &ctx_username = opt_ctx_username.value();

    std::error_code ec;

    auto get = [&]()
    {
        auto api_key = authentication_handler.get_api_key(req.username, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        if(!api_key)
        {
            return tr.send_error("Can't retrieve APIAccessSecretKey.");
        }

        dto::user_api_key_get_success res;
        res.api_key = api_key.value();

        return tr.send(res);
    };

    if(ctx_username == req.username)
    {
        return get();
    }

    auto group = authentication_handler.get_group(ctx_username, ec);

    if(!group || ec)
    {
        return tr.send_error("Internal error.");
    }

    if(group.value() == "admin")
    {
        return get();
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
