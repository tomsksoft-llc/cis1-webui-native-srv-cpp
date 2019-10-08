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
    auto* project = cis_manager.get_project_info(req.project);

    auto perm = rights.check_project_right(ctx.username, req.project);
    auto permitted = perm.has_value() ? perm.value().read : true;

    if(project != nullptr && permitted)
    {
        dto::cis_project_info_success res;

        for(auto& file : project->get_files())
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

        for(auto& [job_name, job] : project->get_jobs())
        {
            auto it = std::find_if(
                    res.fs_entries.begin(),
                    res.fs_entries.end(),
                    [&job_name](auto& entry)
                    {
                        return entry.name == job_name;
                    });

            it->metainfo = dto::fs_entry::job_info{};
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
