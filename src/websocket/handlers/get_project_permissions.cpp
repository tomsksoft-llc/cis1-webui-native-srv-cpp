/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_project_permissions.h"

#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"
#include "websocket/dto/admin_project_permission_get_success.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket::handlers
{

void get_project_permissions(
        cis::cis_manager_interface& cis_manager,
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_project_permission_get& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto& email = ctx.client_info.value().email;

    // check if the user is admin
    const auto is_admin = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(!is_admin)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    // check if the project exists
    if(!cis_manager.get_project_info(req.project))
    {
        return tr.send_error(dto::cis_project_error_doesnt_exist{}, "Project doesn't exists.");
    }

    const auto permissions = rights.get_permissions_by_project(req.project, ec);

    WSHU_CHECK_EC(ec);

    dto::admin_project_permission_get_success res;

    for(const auto &[project_name, project_rights_ex] : permissions)
    {

        res.users.push_back({
                project_name,
                project_rights_ex.admin,
                project_rights_ex.read,
                project_rights_ex.write,
                project_rights_ex.execute});
    }

    return tr.send(res);
}

}
