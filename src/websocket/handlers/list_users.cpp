/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/list_users.h"

#include "websocket/dto/admin_user_list_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void list_users(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_user_list& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto& email = ctx.client_info.value().email;

    const auto is_admin = rights.is_admin(email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!is_admin)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto users = authentication_handler.get_user_infos(ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::admin_user_list_success res;

    for(auto& user : users)
    {
        res.users.push_back({
                user.email,
                user.api_access_key ? user.api_access_key.value() : "",
                user.is_admin});
    }

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
