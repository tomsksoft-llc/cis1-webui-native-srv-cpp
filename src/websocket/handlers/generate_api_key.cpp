/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/generate_api_key.h"

#include "websocket/dto/user_api_key_generate_success.h"
#include "websocket/dto/user_permissions_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void generate_api_key(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_api_key_generate& req,
        cis1::proto_utils::transaction tr)
{
    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.");
    }

    const auto& email = ctx.client_info.value().email;

    std::error_code ec;

    auto generate = [&]()
    {
        auto api_key = authentication_handler.generate_api_key(req.email, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        if(!api_key)
        {
            return tr.send_error("Can't generate APIAccessSecretKey.");
        }

        dto::user_api_key_generate_success res;
        res.api_key = api_key.value();

        return tr.send(res);
    };

    if(email == req.email)
    {
        return generate();
    }

    const auto is_admin = rights.is_admin(req.email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(is_admin)
    {
        return generate();
    }

    return tr.send_error(dto::user_permissions_error_access_denied{}, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
