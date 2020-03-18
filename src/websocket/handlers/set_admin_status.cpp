/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/set_admin_status.h"

#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/user_error_user_not_found.h"
#include "websocket/dto/admin_user_set_admin_status_success.h"

namespace websocket::handlers
{


void set_admin_status(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_user_set_admin_status& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto& email = ctx.client_info.value().email;
    const auto permitted = rights.is_admin(email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!permitted)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto user_exists = authentication_handler.has_user(req.email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!user_exists)
    {
        return tr.send_error(dto::user_error_user_not_found{}, "User does not exist.");
    }

    // check if the user is admin
    const auto success = rights.set_admin_status(req.email, req.admin, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!success)
    {
        // TODO
        return tr.send_error("Internal error.");
    }

    dto::admin_user_set_admin_status_success res;

    return tr.send(res);
}

}
