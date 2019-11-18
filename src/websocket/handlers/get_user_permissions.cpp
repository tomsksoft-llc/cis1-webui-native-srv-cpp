/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_user_permissions.h"

#include "websocket/dto/user_permissions_get_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void get_user_permissions(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_permissions_get& req,
        cis1::proto_utils::transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto permissions = rights.get_permissions(req.username);

        dto::user_permissions_get_success res;

        for(auto [project_name, project_rights] : permissions)
        {
            res.permissions.push_back({
                        project_name,
                        project_rights.read,
                        project_rights.write,
                        project_rights.execute});
        }

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted");
}

} // namespace handlers

} // namespace websocket
