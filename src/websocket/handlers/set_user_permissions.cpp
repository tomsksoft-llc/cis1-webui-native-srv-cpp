#include "websocket/handlers/set_user_permissions.h"

#include "websocket/dto/user_permissions_set_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void set_user_permissions(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_permissions_set& req,
        transaction tr)
{
    auto perm = rights.check_user_permission(ctx.username, "users.permissions");
    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        for(auto& perm : req.permissions)
        {
            rights.set_user_project_permissions(
                    req.username,
                    perm.project,
                    {-1, -1, -1, perm.read, perm.write, perm.execute});
        }

        dto::user_permissions_set_success res;

        return tr.send(res);
    }

    dto::user_permissions_error_access_denied err;

    return tr.send_error(err, "Action not permitted");
}

} // namespace handlers

} // namespace websocket
