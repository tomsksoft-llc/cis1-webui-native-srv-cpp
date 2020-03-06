/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/change_pass.h"

#include "websocket/dto/user_auth_error_pass_doesnt_match.h"
#include "websocket/dto/user_auth_change_pass_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void change_pass(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_change_pass& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_auth_error_pass_doesnt_match{}, "Invalid password");
    }

    const auto change = [&]()
    {
        bool ok = authentication_handler.change_pass(
                req.email,
                req.old_password,
                req.new_password,
                ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        if(!ok)
        {
            return tr.send_error(dto::user_auth_error_pass_doesnt_match{}, "Invalid password");
        }

        dto::user_auth_change_pass_success res;
        return tr.send(res);
    };

    const auto& email = ctx.client_info.value().email;

    // check if requested email is the same or the user is admin
    if(email == req.email)
    {
        return change();
    }

    const auto is_admin = rights.is_admin(email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(is_admin)
    {
        return change();
    }

    return tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
