/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_user_permissions_projects.h"

#include "websocket/dto/user_permissions_projects_get_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void get_user_permissions_projects(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_permissions_projects_get& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    auto perm = rights.check_user_permission(ctx.cln_info, "users.permissions", ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm.has_value() ? perm.value() : false;

    if(permitted)
    {
        const auto permissions = rights.get_projects_permissions(req.username, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        dto::user_permissions_projects_get_success res;

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
