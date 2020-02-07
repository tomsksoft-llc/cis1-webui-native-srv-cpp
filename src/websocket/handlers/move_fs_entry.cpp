/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/move_fs_entry.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_cant_move.h"
#include "websocket/dto/fs_entry_move_success.h"
#include "websocket/dto/auth_error_login_required.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void move_fs_entry(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_move& req,
        cis1::proto_utils::transaction tr)
{
    const auto on_not_permitted =
            [&]()
            {
                return request_context::authorized(ctx.client_info)
                       ? tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.")
                       : tr.send_error(dto::auth_error_login_required{}, "Login required.");
            };

    std::filesystem::path old_path(req.old_path);
    std::filesystem::path new_path(req.new_path);

    if(!validate_path(old_path) || !validate_path(new_path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    std::error_code ec;

    auto path_rights = get_path_rights(ctx, rights, old_path, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!path_rights || !path_rights.value().write)
    {
        return on_not_permitted();
    }

    path_rights = get_path_rights(ctx, rights, new_path, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!path_rights || !path_rights.value().write)
    {
        return on_not_permitted();
    }

    auto& fs = cis_manager.fs();

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
