/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/set_fs_entry_executable_flag.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_set_executable_flag_success.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/check_ec.h"
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
    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Login required");

        return tr.send_error(dto::auth_error_login_required{}, "Login required.");
    }

    const auto& email = ctx.client_info.value().email;

    std::filesystem::path path(req.path);

    if(!validate_path(path) || path == "/")
    {
        WSHU_LOG(scl::Level::Info, R"(Invalid path "%s")", req.path);

        return tr.send_error(dto::fs_entry_error_invalid_path{}, "Invalid path.");
    }

    std::error_code ec;

    auto path_rights = get_path_rights(email, rights, path, ec);

    WSHU_CHECK_EC(ec);

    if(!path_rights || !path_rights.value().write)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
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

        WSHU_CHECK_EC(ec);

        WSHU_LOG(scl::Level::Action,
                 R"(fs entry "%s" became %s)",
                 req.path,
                 req.executable ? "executable" : "non-executable");

        dto::fs_entry_set_executable_flag_success res;

        return tr.send(res);
    }

    WSHU_LOG(scl::Level::Info, R"(Path "%s" doesn't exist)", req.path);

    return tr.send_error(dto::fs_entry_error_doesnt_exist{}, "Path does not exist.");
}

} // namespace handlers

} // namespace websocket
