/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/add_user.h"

#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/admin_user_add_success.h"
#include "websocket/dto/admin_user_add_error_exists.h"
#include "websocket/dto/admin_user_add_error_incorrect_credentials.h"

namespace websocket::handlers
{

void add_user(auth_manager_interface& authentication_handler,
              rights_manager_interface& rights,
              request_context& ctx,
              const dto::admin_user_add& req,
              cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto& email = ctx.client_info->email;
    const auto permitted = rights.is_admin(email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!permitted)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    if(req.email.empty() || req.pass.empty())
    {
        return tr.send_error(dto::admin_user_add_error_incorrect_credentials{}, "Incorrect credentials.");
    }

    const auto user_exists = authentication_handler.has_user(req.email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(user_exists)
    {
        return tr.send_error(dto::admin_user_add_error_exists{}, "User exists already");
    }

    const auto success
            = authentication_handler.add_user(req.email, req.pass, req.admin, ec);
    if(!success || ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::admin_user_add_success res;
    return tr.send(res);
}

} // namespace websocket::handlers
