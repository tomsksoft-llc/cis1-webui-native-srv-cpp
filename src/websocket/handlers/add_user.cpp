/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/add_user.h"

#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/admin_user_add_success.h"
#include "websocket/dto/admin_user_add_error_exists.h"
#include "websocket/dto/admin_user_add_error_incorrect_credentials.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket::handlers
{

void add_user(auth_manager_interface& authentication_handler,
              rights_manager_interface& rights,
              request_context& ctx,
              const dto::admin_user_add& req,
              cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted.");
    }

    const auto& email = ctx.client_info->email;
    const auto permitted = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(!permitted)
    {
        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    if(req.email.empty() || req.pass.empty())
    {
        WSHU_LOG(scl::Level::Info, "Incorrect credentials: email or pass is empty");

        return tr.send_error(dto::admin_user_add_error_incorrect_credentials{}, "Incorrect credentials.");
    }

    const auto user_exists = authentication_handler.has_user(req.email, ec);

    WSHU_CHECK_EC(ec);

    if(user_exists)
    {
        WSHU_LOG(scl::Level::Info, R"(User with the "%s" email exists already)", req.email);

        return tr.send_error(dto::admin_user_add_error_exists{}, "User exists already");
    }

    // TODO check random_pass

    const auto success
            = authentication_handler.add_user(req.email, req.pass, req.admin, ec);
    if(!success || ec)
    {
        WSHU_LOG(scl::Level::Error, "Internal error: %s", ec.message());

        return tr.send_error("Internal error.");
    }

    WSHU_LOG(scl::Level::Action, R"(User "%s" was added)", req.email);

    dto::admin_user_add_success res;
    return tr.send(res);
}

} // namespace websocket::handlers
