#pragma once

#include "request_context.h"
#include "auth_manager_interface.h"
#include "transaction.h"

#include "websocket/dto/auth_token.h"

namespace websocket
{

namespace handlers
{

void token(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::auth_token& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
