/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/get_api_key.h"

#include "websocket/dto/user_api_key_get_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"

namespace websocket
{

namespace handlers
{

void get_api_key(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_api_key_get& req,
        cis1::proto_utils::transaction tr)
{
    if(!ctx.client_info)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    const auto& email = ctx.client_info.value().email;

    std::error_code ec;

    auto get = [&]()
    {
        auto api_key = authentication_handler.get_api_key(req.email, ec);

        if(ec)
        {
            return tr.send_error("Internal error.");
        }

        if(!api_key)
        {
            return tr.send_error("Can't retrieve APIAccessSecretKey.");
        }

        dto::user_api_key_get_success res;
        res.api_key = api_key.value();

        return tr.send(res);
    };

    if(email == req.email)
    {
        return get();
    }

    const auto is_admin = rights.is_admin(email, ec);
    if(ec)
    {
        return tr.send_error("Internal error.");
    }

    if(is_admin)
    {
        return get();
    }

    return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
