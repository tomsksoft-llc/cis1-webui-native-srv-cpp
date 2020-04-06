/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_project_info.h"

#include "websocket/dto/cis_project_info_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/make_dir_entry.h"
#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void get_project_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_info& req,
        cis1::proto_utils::transaction tr)
{
    auto project = cis_manager.get_project_info(req.project);

    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Login required");

        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    std::error_code ec;

    auto perm = rights.check_project_right(email, req.project, ec);

    WSHU_CHECK_EC(ec);

    auto permitted = perm && perm.value().read;

    if(project != nullptr && permitted)
    {
        dto::cis_project_info_success res;

        for(auto& entry : project->get_job_list())
        {
            auto res_entry = std::visit(
                    meta::overloaded{
                        [&](const std::shared_ptr<fs_entry_interface>& entry)
                        {
                            return make_dir_entry(
                                    cis_manager.fs().root().path(),
                                    *entry);
                        },
                        [&](const std::shared_ptr<job_interface>& job)
                        {
                            auto entry = make_dir_entry(
                                    cis_manager.fs().root().path(),
                                    *job);

                            entry.metainfo = dto::fs_entry::job_info{};

                            return entry;
                        }
                    },
                    entry);

            res.fs_entries.push_back(res_entry);
        }

        WSHU_LOG(scl::Level::Action, R"("%s" project info was sent)", req.project);

        return tr.send(res);
    }

    if(!permitted)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    dto::cis_project_error_doesnt_exist err;
    err.project = req.project;

    WSHU_LOG(scl::Level::Info, R"("%s" project doesn't exist)", req.project);

    return tr.send_error(err, "Project doesn't exist.");
}

} // namespace handlers

} // namespace websocket
