/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/add_user.h"

#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/user_auth_add_success.h"
#include "websocket/dto/user_auth_add_error_exists.h"
#include "websocket/dto/user_auth_add_error_incorrect_credentials.h"

namespace websocket::handlers
{

void add_user(auth_manager_interface& authentication_handler,
              rights_manager_interface& rights,
              request_context& ctx,
              const dto::user_auth_add& req,
              cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    const auto perm = rights.check_user_permission(ctx.cln_info, "users.add", ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    const auto permitted = perm && perm.value();
    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;
        return tr.send_error(err, "Action not permitted");
    }

    if(req.username.empty() || req.pass.empty() || req.email.empty())
    {
        dto::user_auth_add_error_incorrect_credentials err;
        return tr.send_error(err, "Incorrect credentials.");
    }

    const auto user_exists = authentication_handler.has_user(req.username, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(user_exists)
    {
        dto::user_auth_add_error_exists err;
        return tr.send_error(err, "User exists already");
    }

    const auto email_exists = authentication_handler.has_email(req.email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(email_exists)
    {
        dto::user_auth_add_error_exists err;
        return tr.send_error(err, "User with the same email exists already");
    }

    const auto success
            = authentication_handler.add_user(req.username, req.pass, req.email, ec);
    if(!success || ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::user_auth_add_success res;
    return tr.send(res);
}

} // namespace
