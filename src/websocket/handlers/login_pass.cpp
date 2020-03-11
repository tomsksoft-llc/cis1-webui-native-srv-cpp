/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/login_pass.h"

#include "websocket/dto/auth_login_pass_success.h"
#include "websocket/dto/auth_error_wrong_credentials.h"

namespace websocket
{

namespace handlers
{

void login_pass(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::auth_login_pass& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    auto token = authentication_handler.authenticate(
            req.email,
            req.pass,
            ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!token)
    {
        return tr.send_error(dto::auth_error_wrong_credentials{}, "Wrong credentials.");
    }

    ctx.client_info = request_context::user_info{req.email, token.value()};

    const auto is_admin = rights.is_admin(req.email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::auth_login_pass_success res;
    res.token = token.value();
    res.admin = is_admin;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket