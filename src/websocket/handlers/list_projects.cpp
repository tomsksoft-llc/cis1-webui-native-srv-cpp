#include "websocket/handlers/list_projects.h"

#include "websocket/dto/cis_project_list_get_success.h"

namespace websocket
{

namespace handlers
{

void list_projects(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_project_list_get& req,
        transaction tr)
{
    dto::cis_project_list_get_success res;

    for(auto& file : cis_manager.fs())
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

    for(auto& [project_name, project] : cis_manager.get_projects())
    {
        auto it = std::find_if(
                res.fs_entries.begin(),
                res.fs_entries.end(),
                [&project_name](auto& entry)
                {
                    return entry.name == project_name;
                });

        bool permitted = false;

        if(auto perm = rights.check_project_right(ctx.username, project_name);
                (perm.has_value() && perm.value().read) || !perm.has_value())
        {
            permitted = true;
        }

        it->metainfo = dto::fs_entry::project_info{true};
    }

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
