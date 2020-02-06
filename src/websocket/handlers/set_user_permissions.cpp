/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/set_user_permissions.h"

#include "websocket/dto/user_permissions_projects_set_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void set_user_permissions_projects(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_permissions_projects_set& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    auto perm = rights.check_user_permission(ctx.client_info, "users.permissions", ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        for(auto& perm : req.permissions)
        {
            rights.set_user_project_permissions(
                    req.username,
                    perm.project,
                    {-1, -1, -1, perm.read, perm.write, perm.execute},
                    ec);

            if(ec)
            {
                return tr.send_error("Internal error.");
            }
        }

        dto::user_permissions_projects_set_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted");
}

} // namespace handlers

} // namespace websocket
