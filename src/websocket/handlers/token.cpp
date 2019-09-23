#include "token.h"

#include "websocket/dto/auth_login_pass_success.h"
#include "websocket/dto/auth_error_wrong_credentials.h"

namespace websocket
{

namespace handlers
{

void token(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_token& req,
        transaction tr)
{
    auto username = authentication_handler.authenticate(req.token);

    if(username)
    {
        ctx.username = username.value();
        ctx.active_token = req.token;

        auto group = authentication_handler.get_group(ctx.username);

        if(group)
        {
            dto::auth_login_pass_success res;
            res.token = req.token;
            res.group = group.value();

            return tr.send(res);
        }

        return tr.send_error("Internal error.");
    }

    dto::auth_error_wrong_credentials err;

    return tr.send_error(err, "Invalid token.");
}

} // namespace handlers

} // namespace websocket
