#pragma once

#include "request_context.h"
#include "auth_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/auth_token.h"

namespace websocket
{

namespace handlers
{

void token(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_token& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
