/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/remove_cis_build.h"

#include "websocket/dto/cis_build_remove_success.h"
#include "websocket/dto/cis_build_error_doesnt_exist.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void remove_cis_build(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_build_remove& req,
        cis1::proto_utils::transaction tr)
{
    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Login required");

        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    auto build = cis_manager.get_build_info(
            req.project,
            req.job,
            req.build);

    std::error_code ec;

    auto perm = rights.check_project_right(email, req.project, ec);

    WSHU_CHECK_EC(ec);

    auto permitted = perm && perm.value().write;

    if(build != nullptr && permitted)
    {
        auto build_path =
                std::filesystem::path{"/"} / req.project / req.job / req.build;

        auto& fs = cis_manager.fs();

        if(auto it = fs.find(build_path); it != fs.end())
        {
            it.remove();

            dto::cis_build_remove_success res;

            WSHU_LOG(scl::Level::Action, R"("%s/%s/%s" build was removed)", req.project, req.job, req.build);

            return tr.send(res);
        }

        WSHU_LOG(scl::Level::Info, R"("%s/%s/%s" build doesn't exist)", req.project, req.job, req.build);

        dto::cis_build_error_doesnt_exist err;
        err.project = req.project;
        err.job = err.job;

        return tr.send_error(err, "Build doesn't exist.");
    }

    if(!permitted)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    dto::cis_build_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;
    err.build = req.build;

    WSHU_LOG(scl::Level::Info, R"("%s/%s/%s" build doesn't exist)", req.project, req.job, req.build);

    return tr.send_error(err, "Build doesn't exists.");
}

} // namespace handlers

} // namespace websocket
