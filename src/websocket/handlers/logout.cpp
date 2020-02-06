/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/logout.h"

#include <tpl_helpers/overloaded.h>

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
    std::error_code ec;

    auto username = authentication_handler.authenticate(req.token, ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    const auto credentials_correct
            = std::visit(
                    meta::overloaded{
                            [&username, &req](const request_context::user_info& cln_ctx)
                            {
                                return username
                                       && username.value() == cln_ctx.username
                                       && req.token == cln_ctx.active_token;
                            },
                            [](const request_context::guest_info& cln_ctx)
                            {
                                return false;
                            }},
                    ctx.client_info);

    if(!credentials_correct)
    {
        dto::auth_error_wrong_credentials err;
        return tr.send_error(err, "Invalid token.");
    }

    authentication_handler.delete_token(req.token, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    dto::auth_logout_success res;
    return tr.send(res);
}

} // namespace handlers

} // namespace websocket
