/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/remove_api_key.h"

#include "websocket/dto/user_api_key_remove_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void remove_api_key(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_remove& req,
        cis1::proto_utils::transaction tr)
{
    const auto opt_ctx_username = request_context::username(ctx.client_info);

    if(!opt_ctx_username)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    const auto &ctx_username = opt_ctx_username.value();

    std::error_code ec;

    auto remove = [&]()
    {
        auto result = authentication_handler.remove_api_key(req.username, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        if(!result)
        {
            return tr.send_error("Can't remove APIAccessSecretKey.");
        }

        dto::user_api_key_remove_success res;

        return tr.send(res);
    };

    if(ctx_username == req.username)
    {
        return remove();
    }

    auto group = authentication_handler.get_group(ctx_username, ec);

    if(!group || ec)
    {
        return tr.send_error("Internal error.");
    }

    if(group.value() == "admin")
    {
        return remove();
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
