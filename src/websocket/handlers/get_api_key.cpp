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

#include "websocket/handlers/utils/check_ec.h"

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
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    const auto& email = ctx.client_info.value().email;

    std::error_code ec;

    auto get = [&]()
    {
        auto api_key = authentication_handler.get_api_key(req.email, ec);

        WSHU_CHECK_EC(ec);

        if(!api_key)
        {
            WSHU_LOG(scl::Level::Info, "Can't retrieve APIAccessSecretKey");

            return tr.send_error("Can't retrieve APIAccessSecretKey.");
        }

        WSHU_LOG(scl::Level::Action, R"(User's "%s" APIAccessSecretKey: %s)", req.email, api_key.value());

        dto::user_api_key_get_success res;
        res.api_key = api_key.value();

        return tr.send(res);
    };

    if(email == req.email)
    {
        return get();
    }

    const auto is_admin = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(is_admin)
    {
        return get();
    }

    WSHU_LOG(scl::Level::Info, "Action not permitted");

    return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
