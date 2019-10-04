#include "websocket/handlers/logout.h"

#include "websocket/dto/auth_logout_success.h"
#include "websocket/dto/auth_error_wrong_credentials.h"

namespace websocket
{

namespace handlers
{

void logout(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::auth_logout& req,
        cis1::proto_utils::transaction tr)
{
    auto username = authentication_handler.authenticate(req.token);

    if(username && username.value() == ctx.username)
    {
        if(req.token == ctx.active_token)
        {
            //TODO clean subs?
            ctx.active_token.clear();
            ctx.username.clear();
        }

        authentication_handler.delete_token(req.token);

        dto::auth_logout_success res;

        return tr.send(res);
    }

    dto::auth_error_wrong_credentials err;

    return tr.send_error(err, "Invalid token.");
}

} // namespace handlers

} // namespace websocket
