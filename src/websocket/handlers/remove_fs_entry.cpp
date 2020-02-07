/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/remove_fs_entry.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_remove_success.h"
#include "websocket/dto/auth_error_login_required.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void remove_fs_entry(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_remove& req,
        cis1::proto_utils::transaction tr)
{
    std::filesystem::path path(req.path);

    if(!validate_path(path) || path == "/")
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    std::error_code ec;

    auto path_rights = get_path_rights(ctx, rights, path, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!path_rights || !path_rights.value().write)
    {
        return request_context::authorized(ctx.client_info)
               ? tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.")
               : tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    auto remove_result = cis_manager.remove(path);

    if(!remove_result)
    {
        dto::fs_entry_error_doesnt_exist err;

        return tr.send_error(err, "Path does not exists.");
    }

    dto::fs_entry_remove_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
