/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/remove_cis_project.h"

#include "websocket/dto/cis_project_remove_success.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/auth_error_login_required.h"

namespace websocket
{

namespace handlers
{

void remove_cis_project(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_remove& req,
        cis1::proto_utils::transaction tr)
{
    auto project = cis_manager.get_project_info(req.project);

    std::error_code ec;

    auto perm = rights.check_project_right(ctx.client_info, req.project, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm && perm.value().write;

    if(project != nullptr && permitted)
    {
        std::error_code ec;

        cis_manager.remove_project(project, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        dto::cis_project_remove_success res;

        return tr.send(res);
    }

    if(!permitted)
    {
        return request_context::authorized(ctx.client_info)
               ? tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.")
               : tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    dto::cis_project_error_doesnt_exist err;
    err.project = req.project;

    return tr.send_error(err, "Project doesn't exists.");
}

} // namespace handlers

} // namespace websocket
