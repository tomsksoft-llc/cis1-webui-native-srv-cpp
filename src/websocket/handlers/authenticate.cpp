/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/authenticate.h"

#include "websocket/dto/auth_login_pass_success.h"
#include "websocket/dto/auth_error_wrong_credentials.h"

namespace websocket
{

namespace handlers
{

void authenticate(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::auth_login_pass& req,
        cis1::proto_utils::transaction tr)
{
    auto token = authentication_handler.authenticate(
            req.username,
            req.pass);

    if(token)
    {
        ctx.username = req.username;
        ctx.active_token = token.value();

        auto group = authentication_handler.get_group(ctx.username);

        if(group)
        {
            dto::auth_login_pass_success res;
            res.token = token.value();
            res.group = group.value();

            return tr.send(res);
        }

        return tr.send_error("Internal error.");
    }

    dto::auth_error_wrong_credentials err;

    return tr.send_error(err, "Wrong credentials.");
}

} // namespace handlers

} // namespace websocket
