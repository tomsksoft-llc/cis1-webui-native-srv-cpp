/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/new_directory.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/fs_entry_error_cant_create_dir.h"
#include "websocket/dto/fs_entry_new_dir_success.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/check_ec.h"
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
    if(!ctx.client_info)
    {
        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    std::filesystem::path path(req.path);

    if(!validate_path(path))
    {
        dto::fs_entry_error_invalid_path err;

        return tr.send_error(err, "Invalid path.");
    }

    std::error_code ec;

    auto path_rights = get_path_rights(email, rights, path, ec);

    WSHU_CHECK_EC(ec);

    if(!path_rights || !path_rights.value().write)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    fs.create_directory(path, ec);

    if(ec)
    {
        return tr.send_error(dto::fs_entry_error_cant_create_dir{}, "Error while creating directory.");
    }

    dto::fs_entry_new_dir_success res;
    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
