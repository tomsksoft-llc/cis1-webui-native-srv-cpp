/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/login_token.h"

#include "websocket/dto/auth_login_success.h"
#include "websocket/dto/auth_error_wrong_credentials.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void login_token(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::auth_login_token& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    auto email = authentication_handler.authenticate(req.token, ec);

    WSHU_CHECK_EC(ec);

    if(!email)
    {
        WSHU_LOG(scl::Level::Info, "Invalid token");

        return tr.send_error(dto::auth_error_wrong_credentials{}, "Invalid token.");
    }

    ctx.client_info = request_context::user_info{email.value(), req.token};

    const auto is_admin = rights.is_admin(email.value(), ec);

    WSHU_CHECK_EC(ec);
    WSHU_LOG(scl::Level::Action, "%s logged in", is_admin ? "Admin" : "User");

    dto::auth_login_success res;
    res.token = req.token;
    res.admin = is_admin;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
