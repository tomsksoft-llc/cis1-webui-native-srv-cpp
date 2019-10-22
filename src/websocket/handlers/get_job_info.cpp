#include "websocket/handlers/get_job_info.h"

#include "websocket/dto/cis_job_info_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_job_error_doesnt_exist.h"

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
    auto* job = cis_manager.get_job_info(req.project, req.job);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(job != nullptr && permitted)
    {
        dto::cis_job_info_success res;

        for(auto& file : job->get_files())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).generic_string();
            auto link = ("/download" / file.relative_path()).generic_string();

            res.fs_entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
        }

        for(auto& param : job->get_params())
        {
            res.params.push_back({param.name, param.default_value});
        }

        for(auto& [build_name, build] : job->get_builds())
        {
            auto& info = build.get_info();

            auto it = std::find_if(
                    res.fs_entries.begin(),
                    res.fs_entries.end(),
                    [&build_name](auto& entry)
                    {
                        return entry.name == build_name;
                    });

            it->metainfo = dto::fs_entry::build_info{
                    info.status,
                    info.date};
        }

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
