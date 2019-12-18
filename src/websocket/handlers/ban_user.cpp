/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/ban_user.h"

#include "websocket/dto/user_auth_ban_success.h"
#include "websocket/dto/user_auth_error_user_not_found.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void ban_user(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_auth_ban& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    auto user_exists = authentication_handler.has_user(req.username, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!user_exists)
    {
        dto::user_auth_error_user_not_found err;

        return tr.send_error(err, "Invalid username.");
    }

    auto perm = rights.check_user_permission(ctx.username, "users.change_group", ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(
                req.username,
                "disabled",
                ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        dto::user_auth_ban_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
