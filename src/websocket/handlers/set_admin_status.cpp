/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#include "websocket/handlers/set_admin_status.h"

#include "websocket/dto/user_permission_error_access_denied.h"
#include "websocket/dto/user_error_user_not_found.h"
#include "websocket/dto/admin_user_set_admin_status_success.h"

#include "websocket/handlers/utils/check_ec.h"

namespace websocket::handlers
{


void set_admin_status(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_user_set_admin_status& req,
        cis1::proto_utils::transaction tr)
{
    std::error_code ec;

    if(!ctx.client_info)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto& email = ctx.client_info.value().email;
    const auto permitted = rights.is_admin(email, ec);

    WSHU_CHECK_EC(ec);

    if(!permitted)
    {
        WSHU_LOG(scl::Level::Info, "Action not permitted");

        return tr.send_error(dto::user_permission_error_access_denied{}, "Action not permitted");
    }

    const auto user_exists = authentication_handler.has_user(req.email, ec);

    WSHU_CHECK_EC(ec);

    if(!user_exists)
    {
        WSHU_LOG(scl::Level::Info, R"(User "%s" doesn't exist)", req.email);

        return tr.send_error(dto::user_error_user_not_found{}, "User does not exist.");
    }

    // check if the user is admin
    const auto success = rights.set_admin_status(req.email, req.admin, ec);
    if(ec || !success)
    {
        WSHU_LOG(scl::Level::Error, "Internal error: %s", ec.message());

        return tr.send_error("Internal error.");
    }

    WSHU_LOG(scl::Level::Action,
             R"("%s" became %s)",
             req.email,
             req.admin ? "Admin" : "User");

    dto::admin_user_set_admin_status_success res;

    return tr.send(res);
}

}
