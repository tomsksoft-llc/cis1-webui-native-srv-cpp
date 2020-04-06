/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/set_user_permissions.h"

#include "websocket/dto/admin_user_permission_set_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void set_user_permissions(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_user_permission_set& req,
        cis1::proto_utils::transaction tr)
{
    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto& email = ctx.client_info.value().email;

    std::error_code ec;

    const auto is_admin = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(!is_admin)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    for(auto& perm : req.permissions)
    {
        rights.set_user_project_permissions(
                req.email,
                perm.project,
                {-1, -1, -1, perm.read, perm.write, perm.execute},
                ec);

        WSHU_CHECK_EC(ec);
        WSHU_LOG(scl::Level::Action,
                 R"(Permissions of "%s" were set: %b %b %b)",
                 req.email,
                 perm.read,
                 perm.write,
                 perm.execute);
    }

    dto::admin_user_permission_set_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
