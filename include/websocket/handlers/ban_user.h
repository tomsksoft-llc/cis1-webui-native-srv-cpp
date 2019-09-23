#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "auth_manager_interface.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_auth_ban.h"

namespace websocket
{

namespace handlers
{

void ban_user(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_auth_ban& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
