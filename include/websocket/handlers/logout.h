#pragma once

#include "request_context.h"
#include "auth_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/auth_logout.h"

namespace websocket
{

namespace handlers
{

void logout(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_logout& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
