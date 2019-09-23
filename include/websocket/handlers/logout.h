#pragma once

#include "request_context.h"
#include "auth_manager_interface.h"
#include "websocket/transaction.h"

#include "websocket/dto/auth_logout.h"

namespace websocket
{

namespace handlers
{

void logout(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::auth_logout& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
