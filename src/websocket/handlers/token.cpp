/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/token.h"

#include "websocket/dto/auth_login_pass_success.h"
#include "websocket/dto/auth_error_wrong_credentials.h"

namespace websocket
{

namespace handlers
{

void token(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::auth_token& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    auto username = authentication_handler.authenticate(req.token, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(!username)
    {
        dto::auth_error_wrong_credentials err;
        return tr.send_error(err, "Invalid token.");
    }

    ctx.cln_info = request_context::user_info{username.value(), req.token};

    auto group = authentication_handler.get_group(username.value(), ec);
    if(!group || ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::auth_login_pass_success res;
    res.token = req.token;
    res.group = group.value();

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
