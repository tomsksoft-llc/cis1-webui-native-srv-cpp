/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/set_fs_entry_executable_flag.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permissions_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_set_executable_flag_success.h"

#include "path_utils.h"

namespace websocket
{

namespace handlers
{

void set_fs_entry_executable_flag(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_set_executable_flag& req,
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
        dto::user_permissions_error_access_denied err;

        return tr.send_error(err, "Action not permitted.");
    }

    auto& fs = cis_manager.fs();

    if(auto it = fs.find(path); it != fs.end())
    {
        auto& entry = *it;

        std::error_code ec;

        std::filesystem::permissions(
            entry.path(),
            std::filesystem::perms::owner_exec,
            req.executable
                ? std::filesystem::perm_options::add
                : std::filesystem::perm_options::remove,
            ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        dto::fs_entry_set_executable_flag_success res;

        return tr.send(res);
    }

    dto::fs_entry_error_doesnt_exist err;

    return tr.send_error(err, "Path does not exists.");
}

} // namespace handlers

} // namespace websocket
