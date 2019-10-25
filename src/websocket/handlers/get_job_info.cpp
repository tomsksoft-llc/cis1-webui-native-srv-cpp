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
    auto job = cis_manager.get_job_info(req.project, req.job);

    auto perm = rights.check_project_right(ctx.username, req.project);
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
            auto make_dir_entry = [&](const std::shared_ptr<fs_entry_interface>& entry)
            {
                auto& file = entry->dir_entry();

                bool is_directory = file.is_directory();

                auto relative_path = file.path().lexically_relative(
                        cis_manager.fs().root().path());

                auto path = ("/" / relative_path)
                        .generic_string();

                auto link = "/download" + path;

                return dto::fs_entry{
                        file.path().filename(),
                        false,
                        is_directory,
                        path,
                        link};
            };

            auto res_entry = std::visit(
                    meta::overloaded{
                        make_dir_entry,
                        [&](const std::shared_ptr<build_interface>& build)
                        {
                            auto entry = make_dir_entry(build);

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
