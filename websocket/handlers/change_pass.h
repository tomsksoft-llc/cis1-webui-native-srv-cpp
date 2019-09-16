#pragma once

#include "request_context.h"
#include "auth_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_auth_change_pass.h"

namespace websocket
{

namespace handlers
{

void change_pass(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_auth_change_pass& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
