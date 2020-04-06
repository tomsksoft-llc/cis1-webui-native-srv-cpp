/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_user_permissions.h"

#include "websocket/dto/admin_user_permission_get_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void get_user_permissions(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_user_permission_get& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto& email = ctx.client_info.value().email;

    const auto is_admin = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(!is_admin)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto permissions = rights.get_permissions(req.email, ec);

    WSHU_CHECK_EC(ec);

    dto::admin_user_permission_get_success res;

    for(const auto &[project_name, project_rights] : permissions)
    {
        res.permissions.push_back({
                    project_name,
                    project_rights.read,
                    project_rights.write,
                    project_rights.execute});
    }

    WSHU_LOG(scl::Level::Action, R"("Permissions of %s" users were sent)", req.email);

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
