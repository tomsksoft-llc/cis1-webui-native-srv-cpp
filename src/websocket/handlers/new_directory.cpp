#include "websocket/handlers/new_directory.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_cant_create_dir.h"
#include "websocket/dto/fs_entry_new_dir_success.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void new_directory(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_new_dir& req,
        cis1::proto_utils::transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, path);
            path_rights && !path_rights.value().write)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    std::error_code ec;
    fs.create_directory(path, ec);

    if(ec)
    {
        dto::fs_entry_error_cant_create_dir err;

        return tr.send_error(err, "Error while creating directory.");
    }

    dto::fs_entry_new_dir_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
