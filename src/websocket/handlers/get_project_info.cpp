#include "websocket/handlers/get_project_info.h"

#include "websocket/dto/cis_project_info_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/cis_project_error_doesnt_exist.h"
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

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project != nullptr && permitted)
    {
        dto::cis_project_info_success res;

        for(auto& entry : project->get_job_list())
        {
            auto make_dir_entry =
            [&](const std::shared_ptr<fs_entry_interface>& entry)
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
                        [&](const std::shared_ptr<job_interface>& job)
                        {
                            auto entry = make_dir_entry(job);

                            entry.metainfo = dto::fs_entry::job_info{};

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

    dto::cis_project_error_doesnt_exist err;
    err.project = req.project;

    return tr.send_error(err, "Project doesn't exists.");
}

} // namespace handlers

} // namespace websocket
