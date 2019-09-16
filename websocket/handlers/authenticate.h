#pragma once

#include "request_context.h"
#include "auth_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/auth_login_pass.h"

namespace websocket
{

namespace handlers
{

void authenticate(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::auth_login_pass& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
