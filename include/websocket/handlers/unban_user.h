#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "auth_manager_interface.h"
#include "transaction.h"

#include "websocket/dto/user_auth_unban.h"

namespace websocket
{

namespace handlers
{

void unban_user(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_auth_unban& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
