#include "change_pass.h"

#include "websocket/dto/user_auth_error_pass_doesnt_match.h"
#include "websocket/dto/user_auth_change_pass_success.h"

namespace websocket
{

namespace handlers
{

void change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_auth_change_pass& req,
        transaction tr)
{
    bool ok = authentication_handler.change_pass(
            ctx.username,
            req.old_password,
            req.new_password);

    if(!ok)
    {
        dto::user_auth_error_pass_doesnt_match err;

        return tr.send_error(err, "Invalid password");
    }

    dto::user_auth_change_pass_success res;

    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
