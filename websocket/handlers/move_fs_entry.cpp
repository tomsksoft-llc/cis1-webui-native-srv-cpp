#include "move_fs_entry.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_cant_move.h"
#include "websocket/dto/fs_entry_move_success.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void move_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_move& req,
        transaction tr)
{

    std::filesystem::path old_path(req.old_path);
    std::filesystem::path new_path(req.new_path);

    if(!validate_path(old_path) || !validate_path(new_path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, old_path);
            path_rights && !path_rights.value().write)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    if(auto path_rights = get_path_rights(ctx, rights, new_path);
            path_rights && !path_rights.value().write)
    {
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    std::error_code ec;
    fs.move_entry(old_path, new_path, ec);

    if(ec)
    {
        dto::fs_entry_error_cant_move err;

        return tr.send_error(err, "Error on move.");
    }

    dto::fs_entry_move_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
