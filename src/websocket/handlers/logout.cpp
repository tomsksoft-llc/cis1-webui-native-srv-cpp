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

#include "websocket/handlers/utils/check_ec.h"

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

    if(!ctx.client_info)
    {
        return tr.send_error(dto::auth_error_wrong_credentials{}, "Invalid token.");
    }

    auto email = authentication_handler.authenticate(req.token, ec);

    WSHU_CHECK_EC(ec);

    if(email && email.value() == ctx.client_info.value().email)
    {
        if(req.token == ctx.client_info.value().active_token)
        {
            ctx.client_info.reset();
        }

        authentication_handler.delete_token(req.token, ec);

        WSHU_CHECK_EC(ec);

        dto::auth_logout_success res;

        return tr.send(res);
    }

    dto::auth_error_wrong_credentials err;

    return tr.send_error(err, "Invalid token.");
}

} // namespace handlers

} // namespace websocket
