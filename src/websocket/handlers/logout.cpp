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

    const auto clear_client_info
            = std::visit(
                    meta::overloaded{
                            [&username, &req, &authentication_handler, &tr]
                                    (const request_context::user_info& cln_ctx)
                            {
                                if(!username || username.value() != cln_ctx.username)
                                {
                                    dto::auth_error_wrong_credentials err;
                                    tr.send_error(err, "Invalid token.");
                                    // do not clear the client info
                                    return false;
                                }

                                std::error_code ec;
                                authentication_handler.delete_token(req.token, ec);
                                if(ec)
                                {
                                    tr.send_error("Internal error.");
                                    // do not clear the client info because of the error
                                    return false;
                                }

                                // send a success result
                                dto::auth_logout_success res;
                                tr.send(res);

                                // clear the client info only if the requested token equals the active token
                                return req.token == cln_ctx.active_token;
                            },
                            [&tr](const request_context::guest_info& cln_ctx)
                            {
                                dto::auth_error_wrong_credentials err;
                                tr.send_error(err, "Invalid token.");

                                // the client is guest
                                // therefore do not clear the client info
                                return false;
                            }},
                    ctx.client_info);

    if(clear_client_info)
    {
        ctx.client_info = request_context::guest_info{};
    }
}

} // namespace handlers

} // namespace websocket
