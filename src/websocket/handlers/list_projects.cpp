#include "websocket/handlers/list_projects.h"

#include "websocket/dto/cis_project_list_get_success.h"

#include "cis/dirs.h"

namespace websocket
{

namespace handlers
{

void list_projects(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_list_get& req,
        cis1::proto_utils::transaction tr)
{
    dto::cis_project_list_get_success res;

    for(auto& entry : cis_manager.get_project_list())
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
                    [&](const std::shared_ptr<project_interface>& project)
                    {
                        auto entry = make_dir_entry(project);

                        bool permitted = false;

                        if(auto perm = rights.check_project_right(ctx.username, entry.name);
                                (perm.has_value() && perm.value().read) || !perm.has_value())
                        {
                            permitted = true;
                        }

                        entry.metainfo = dto::fs_entry::project_info{permitted};

                        return entry;
                    }
                },
                entry);

        res.fs_entries.push_back(res_entry);
    }

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
