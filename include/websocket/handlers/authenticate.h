#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "auth_manager_interface.h"

#include "websocket/dto/auth_login_pass.h"

namespace websocket
{

namespace handlers
{

void authenticate(
        auth_manager_interface& authentication_handler,
        request_context& ctx,
        const dto::auth_login_pass& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
