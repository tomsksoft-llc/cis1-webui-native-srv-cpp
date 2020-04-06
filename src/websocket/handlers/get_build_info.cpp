/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_build_info.h"

#include "websocket/dto/cis_build_info_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/cis_build_error_doesnt_exist.h"
#include "websocket/dto/auth_error_login_required.h"
#include "websocket/handlers/utils/make_dir_entry.h"
#include "cis/cis_structs.h"

#include "websocket/handlers/utils/unpack_build_info.h"
#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void get_build_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_build_info& req,
        cis1::proto_utils::transaction tr)
{
    auto build = cis_manager.get_build_info(
            req.project,
            req.job,
            req.build);

    std::error_code ec;

    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Login required");

        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;
    auto perm = rights.check_project_right(email, req.project, ec);

    WSHU_CHECK_EC(ec);

    auto permitted = perm && perm.value().read;

    if(build != nullptr && permitted)
    {
        auto& info = build->get_info();

        auto res = utils::unpack_build_info<dto::cis_build_info_success>(info);

        for(auto it  = build->get_files().begin();
                 it != build->get_files().end();
                 ++it)
        {
            cis::fs_entry_ref entry(it);
            res.fs_entries.push_back(
                    make_dir_entry(
                            cis_manager.fs().root().path(),
                            entry));
        }

        WSHU_LOG(scl::Level::Action, R"("%s/%s/%s" build info was sent)", req.project, req.job, req.build);

        return tr.send(res);
    }

    if(!permitted)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    WSHU_LOG(scl::Level::Info, R"(Build "%s/%s/%s" doesn't exist)", req.project, req.job, req.build);

    dto::cis_build_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;
    err.build = req.build;

    return tr.send_error(err, "Build doesn't exist.");
}

} // namespace handlers

} // namespace websocket
