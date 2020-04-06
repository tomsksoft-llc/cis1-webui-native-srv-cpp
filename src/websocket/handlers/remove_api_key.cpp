/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "websocket/handlers/remove_api_key.h"

#include "websocket/dto/user_api_key_remove_success.h"
#include "websocket/dto/user_permission_error_access_denied.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket
{

namespace handlers
{

void remove_api_key(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_api_key_remove& req,
        cis1::proto_utils::transaction tr)
{
    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    const auto& email = ctx.client_info.value().email;

    std::error_code ec;

    auto remove = [&]()
    {
        auto result = authentication_handler.remove_api_key(req.email, ec);

        WSHU_CHECK_EC(ec);

        if(!result)
        {
            WSHU_LOG(scl::Level::Info, R"(Can't remove APIAccessSecretKey for user "%s")", req.email);

            return tr.send_error("Can't remove APIAccessSecretKey.");
        }

        WSHU_LOG(scl::Level::Action, R"(APIAccessSecretKey was removed for user "%s")", req.email);

        dto::user_api_key_remove_success res;

        return tr.send(res);
    };

    if(email == req.email)
    {
        return remove();
    }

    const auto is_admin = rights.is_admin(email, ec);
    WSHU_CHECK_EC(ec);

    if(is_admin)
    {
        return remove();
    }

    WSHU_LOG(scl::Level::Info, "Action not permitted");

    return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
}

} // namespace handlers

} // namespace websocket
