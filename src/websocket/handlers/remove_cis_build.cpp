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
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/auth_error_login_required.h"

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
    auto build = cis_manager.get_build_info(
            req.project,
            req.job,
            req.build);

    std::error_code ec;

    auto perm = rights.check_project_right(ctx.client_info, req.project, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

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

            return tr.send(res);
        }

        dto::cis_build_error_doesnt_exist err;
        err.project = req.project;
        err.job = err.job;

        return tr.send_error(err, "Build doesn't exists.");
    }

    if(!permitted)
    {
        return request_context::authorized(ctx.client_info)
               ? tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.")
               : tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    dto::cis_build_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;
    err.build = req.build;

    return tr.send_error(err, "Build doesn't exists.");
}

} // namespace handlers

} // namespace websocket
