/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include <tpl_helpers/overloaded.h>
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
    std::error_code ec;

    const auto username = std::visit(
            meta::overloaded{
                    [](const request_context::user_info& ctx) -> std::optional<std::string>
                    { return ctx.username; },
                    [](const request_context::guest_info& ctx) -> std::optional<std::string>
                    { return std::nullopt; }
            },
            ctx.cln_info
    );

    if(!username)
    {
        dto::user_auth_error_pass_doesnt_match err;
        return tr.send_error(err, "Invalid password");
    }

    bool ok = authentication_handler.change_pass(
            username.value(),
            req.old_password,
            req.new_password,
            ec);

    if(ec)
    {
        return tr.send_error("Internal error.");
    }

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
