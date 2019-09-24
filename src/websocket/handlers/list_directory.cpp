#include "websocket/handlers/list_directory.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_list_success.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void list_directory(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_list& req,
        transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().read)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    if(auto it = fs.find(path); it != fs.end())
    {
        dto::fs_entry_list_success res;

        for(auto& file : it->childs())
        {
            bool is_directory = file.dir_entry().is_directory();
            auto path = ("/" / file.relative_path()).generic_string();
            auto link = ("/download" / file.relative_path()).generic_string();

            res.entries.push_back(dto::fs_entry{
                    file.filename(),
                    false,
                    is_directory,
                    path,
                    link});
        }

        return tr.send(res);
    }

    dto::fs_entry_error_doesnt_exist err;

    tr.send_error(err, "Path does not exists.");
}

} // namespace handlers

} // namespace websocket
