/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_job_info.h"

#include "websocket/dto/cis_job_info_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/make_dir_entry.h"
#include "websocket/handlers/utils/unpack_build_info.h"
#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void get_job_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_job_info& req,
        cis1::proto_utils::transaction tr)
{
    auto job = cis_manager.get_job_info(req.project, req.job);

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

    if(job != nullptr && permitted)
    {
        dto::cis_job_info_success res;

        for(auto& param : job->get_params())
        {
            res.params.push_back({param.name, param.default_value});
        }

        for(auto& entry : job->get_build_list())
        {
            auto res_entry = std::visit(
                    meta::overloaded{
                        [&](const std::shared_ptr<fs_entry_interface>& entry)
                        {
                                return make_dir_entry(
                                        cis_manager.fs().root().path(),
                                        *entry);
                        },
                        [&](const std::shared_ptr<build_interface>& build)
                        {
                            auto entry = make_dir_entry(
                                    cis_manager.fs().root().path(),
                                    *build);

                            auto& info = build->get_info();

                            entry.metainfo =
                                    utils::unpack_build_info<dto::fs_entry::build_info>(info);

                            return entry;
                        }
                    },
                    entry);

            res.fs_entries.push_back(res_entry);
        }

        std::reverse(res.fs_entries.begin(), res.fs_entries.end());

        std::stable_sort(
                res.fs_entries.begin(),
                res.fs_entries.end(),
                [](const dto::fs_entry& lhs, const dto::fs_entry& rhs)
                {
                    return std::holds_alternative<dto::fs_entry::build_info>(
                            lhs.metainfo)
                           && !std::holds_alternative<dto::fs_entry::build_info>(
                            rhs.metainfo);
                });

        if(auto script = job->get_script_entry(); script != nullptr)
        {
            res.properties.push_back(
                    make_dir_entry(
                            cis_manager.fs().root().path(),
                            *script));
        }

        if(auto entry = job->get_params_entry(); entry != nullptr)
        {
            res.properties.push_back(
                    make_dir_entry(
                            cis_manager.fs().root().path(),
                            *entry));
        }

        res.properties.push_back(
                make_dir_entry(
                        cis_manager.fs().root().path(),
                        *job->get_conf_entry()));

        WSHU_LOG(scl::Level::Action, R"("%s/%s" job info was sent)", req.project, req.job);

        return tr.send(res);
    }

    if(!permitted)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    WSHU_LOG(scl::Level::Info, R"("%s/%s" job doesn't exist)", req.project, req.job);

    dto::cis_job_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exist.");
}

} // namespace handlers

} // namespace websocket
