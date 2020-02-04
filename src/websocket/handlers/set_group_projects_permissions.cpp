/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/set_group_projects_permissions.h"

#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/group_projects_permissions_set_succuss.h"
#include "websocket/dto/group_error_doesnt_exist.h"

namespace websocket::handlers
{

void set_group_projects_permissions(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::group_projects_permissions_set& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    const auto perm = rights.check_user_permission(ctx.cln_info, "groups.projects.permissions", ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    const auto permitted = perm ? perm.value() : false;
    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;
        return tr.send_error(err, "Action not permitted");
    }

    const auto opt_group_info
            = authentication_handler.get_group_info(req.group, ec);
    if(!opt_group_info)
    {
        const dto::group_error_doesnt_exist err;
        return tr.send_error(err, "Group does not exists.");
    }

    const auto group_id = opt_group_info->id;
    if(!rights.set_group_projects_permissions(group_id, {req.read, req.write, req.execute}, ec)
       || ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::group_projects_permissions_set_success res;
    return tr.send(res);
}

}
