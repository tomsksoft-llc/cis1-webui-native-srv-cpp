/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/remove_fs_entry.h"

#include "websocket/dto/fs_entry_error_invalid_path.h"
#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/fs_entry_error_doesnt_exist.h"
#include "websocket/dto/fs_entry_remove_success.h"
#include "websocket/dto/auth_error_login_required.h"

#include "websocket/handlers/utils/check_ec.h"
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

    auto remove_result = cis_manager.remove(path);

    if(!remove_result)
    {
        WSHU_LOG(scl::Level::Info, R"(Path "%s" does not exist)", req.path);

        return tr.send_error(dto::fs_entry_error_doesnt_exist{}, "Path does not exist.");
    }

    WSHU_LOG(scl::Level::Action, R"(fs entry "%s" was removed)", req.path);

    dto::fs_entry_remove_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
