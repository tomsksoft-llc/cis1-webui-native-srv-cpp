#pragma once

#include "request_context.h"
#include "auth_manager_interface.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_auth_change_pass.h"

namespace websocket
{

namespace handlers
{

void change_pass(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::user_auth_change_pass& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
