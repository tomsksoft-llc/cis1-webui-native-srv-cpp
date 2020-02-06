/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_user_permissions.h"

#include <tpl_helpers/overloaded.h>
#include "websocket/dto/user_permissions_get_error_doesnt_exist.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/user_permissions_get_success.h"

namespace websocket::handlers
{

void get_user_permissions(auth_manager_interface& authentication_handler,
                          rights_manager_interface& rights,
                          request_context& ctx,
                          const dto::user_permissions_get& req,
                          cis1::proto_utils::transaction tr)
{
    std::string requested_username = req.username;

    std::error_code ec;
    const auto perm = rights.check_user_permission(ctx.client_info, "users.permissions", ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    const bool has_permission = perm && perm.value();
    if(has_permission)
    {
        // admin has requested permissions of user with the requested_username
        const auto user_exists = authentication_handler.has_user(requested_username, ec);
        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        if(!user_exists)
        {
            dto::user_permissions_get_error_doesnt_exist err;
            return tr.send_error(err, "Invalid username.");
        }
    }
    else
    {
        const bool permitted = std::visit(
                meta::overloaded{
                        [&requested_username](const request_context::user_info& ctx)
                        {
                            // the user is authorized and has not the "users.permissions" permission,
                            // then the request username should equal the context username
                            return ctx.username == requested_username;
                        },
                        [&requested_username](const request_context::guest_info& ctx)
                        {
                            // the user is guest, therefore the requested username should be empty
                            if(requested_username.empty())
                            {
                                // later we will get permissions for the user with "guest" username
                                requested_username = ctx.guestname;
                                return true;
                            }

                            return false;
                        }
                },
                ctx.client_info);

        if(!permitted)
        {
            dto::user_permissions_error_access_denied err;
            return tr.send_error(err, "Action not permitted");
        }
    }

    const auto permissions = rights.get_user_permissions(requested_username, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::user_permissions_get_success res;

    for(const auto& permission : permissions)
    {
        res.permissions.push_back(
                dto::user_permissions_get_success::permission{permission});
    }

    tr.send(res);
}

} // namespace websocket::handlers
