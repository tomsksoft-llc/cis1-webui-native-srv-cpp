#pragma once

#include "request_context.h"
#include "auth_manager_interface.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_api_key_generate.h"

namespace websocket
{

namespace handlers
{

void generate_api_key(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_generate& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
