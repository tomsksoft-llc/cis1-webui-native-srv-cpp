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
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/check_ec.h"

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
    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Login required");

        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    auto project = cis_manager.get_project_info(req.project);

    std::error_code ec;

    auto perm = rights.check_project_right(email, req.project, ec);

    WSHU_CHECK_EC(ec);

    auto permitted = perm && perm.value().write;

    if(project != nullptr && permitted)
    {
        std::error_code ec;

        cis_manager.remove_project(project, ec);

        WSHU_CHECK_EC(ec);

        dto::cis_project_remove_success res;

        return tr.send(res);
    }

    if(!permitted)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    WSHU_LOG(scl::Level::Info, R"("%s" project doesn't exist)", req.project);

    dto::cis_project_error_doesnt_exist err;
    err.project = req.project;

    return tr.send_error(err, "Project doesn't exists.");
}

} // namespace handlers

} // namespace websocket
