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

namespace websocket
{

namespace handlers
{

void change_pass(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::user_auth_change_pass& req,
        cis1::proto_utils::transaction tr)
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
