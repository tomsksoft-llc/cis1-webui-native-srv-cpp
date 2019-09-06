#pragma once

#include "request_context.h"
#include "auth_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_api_key_get.h"

namespace websocket
{

namespace handlers
{

void get_api_key(
        auth_manager& authentication_handler,
        request_context& ctx,
        const dto::user_api_key_get& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
