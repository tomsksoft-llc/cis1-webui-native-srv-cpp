/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_job_info.h"

#include "websocket/dto/cis_job_info_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"

#include "websocket/handlers/utils/make_dir_entry.h"

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

    std::error_code ec;

    auto perm = rights.check_project_right(ctx.username, req.project, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    auto permitted = perm.has_value() ? perm.value().read : true;

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

                            entry.metainfo = dto::fs_entry::build_info{
                                    info.status,
                                    info.date};

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
                    if(std::holds_alternative<dto::fs_entry::build_info>(
                            lhs.metainfo)
                    && !std::holds_alternative<dto::fs_entry::build_info>(
                            rhs.metainfo))
                    {
                        return true;
                    }

                    return false;
                });

        res.properties.push_back(
                make_dir_entry(
                        cis_manager.fs().root().path(),
                        *job->get_script_entry()));

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

        return tr.send(res);
    }

    if(!permitted)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    dto::cis_job_error_doesnt_exist err;
    err.project = req.project;
    err.job = req.job;

    return tr.send_error(err, "Job doesn't exists.");
}

} // namespace handlers

} // namespace websocket
