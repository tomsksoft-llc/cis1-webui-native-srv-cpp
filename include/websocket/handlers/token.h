#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "auth_manager_interface.h"

#include "websocket/dto/auth_token.h"

namespace websocket
{

namespace handlers
{

void token(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::auth_token& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
