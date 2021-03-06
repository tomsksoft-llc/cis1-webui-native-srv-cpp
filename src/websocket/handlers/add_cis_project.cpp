/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/add_cis_project.h"

#include "websocket/dto/cis_project_add_success.h"
#include "websocket/dto/cis_project_error_already_exist.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void add_cis_project(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_add& req,
        cis1::proto_utils::transaction tr)
{
    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Login required");

        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    auto project = cis_manager.get_project_info(req.project);

    if(project != nullptr)
    {
        dto::cis_project_error_already_exist err;
        err.project = req.project;

        WSHU_LOG(scl::Level::Info, R"(Project "%s" already exists)", req.project);

        return tr.send_error(err, "Project already exists.");
    }

    std::error_code ec;

    const auto is_admin = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(!is_admin)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    cis_manager.create_project(req.project, ec);

    WSHU_CHECK_EC(ec);
    WSHU_LOG(scl::Level::Action, R"(Project "%s" was added)", req.project);

    dto::cis_project_add_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
