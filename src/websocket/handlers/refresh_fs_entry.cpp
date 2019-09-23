#include "websocket/handlers/refresh_fs_entry.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_refresh_success.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void refresh_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_refresh& req,
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

    auto refresh_result = cis_manager.refresh(path);

    if(!refresh_result)
    {
        dto::fs_entry_error_doesnt_exist err;

        return tr.send_error(err, "Path does not exists.");
    }

    dto::fs_entry_refresh_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
