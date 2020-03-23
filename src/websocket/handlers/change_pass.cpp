/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/change_pass.h"

#include "websocket/dto/user_error_pass_doesnt_match.h"
#include "websocket/dto/user_change_pass_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"

#include "websocket/handlers/utils/check_ec.h"

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
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto change = [&]()
    {
        bool ok = authentication_handler.change_pass(
                req.email,
                req.new_password,
                ec);

        WSHU_CHECK_EC(ec);

        if(!ok)
        {
            tr.send_error("Can't change password.");
        }

        dto::user_change_pass_success res;
        return tr.send(res);
    };

    const auto& email = ctx.client_info.value().email;

    const auto is_admin = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(is_admin)
    {
        return change();
    }

    if(email != req.email)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    bool is_same_pass = authentication_handler.check_pass(email, req.old_password);

    WSHU_CHECK_EC(ec);

    if(is_same_pass)
    {
        return change();
    }

    return tr.send_error(dto::user_error_pass_doesnt_match{}, "Invalid password");
}

} // namespace handlers

} // namespace websocket
