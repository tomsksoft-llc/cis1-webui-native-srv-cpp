/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/change_group.h"

#include "websocket/dto/user_auth_change_group_success.h"
#include "websocket/dto/user_auth_error_user_not_found.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void change_group(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_auth_change_group& req,
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

    auto perm = rights.check_user_permission(ctx.cln_info, "users.change_group", ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        authentication_handler.change_group(req.username, req.group, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        dto::user_auth_change_group_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
