/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_group_default_permissions.h"

#include "websocket/dto/group_error_doesnt_exist.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/group_default_permissions_get_success.h"

namespace websocket::handlers
{

void get_group_default_permissions(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::group_default_permissions_get& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    const auto perm = rights.check_user_permission(ctx.client_info, "groups.projects.permissions", ec);

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
    const auto permissions = rights.get_group_default_permissions(group_id, ec);
    if(!permissions || ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::group_default_permissions_get_success res{req.group,
                                                   permissions->read,
                                                   permissions->write,
                                                   permissions->execute};
    return tr.send(res);
}

} // namespace websocket::handlers
