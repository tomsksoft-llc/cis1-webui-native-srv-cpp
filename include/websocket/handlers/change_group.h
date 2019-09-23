#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "auth_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_auth_change_group.h"

namespace websocket
{

namespace handlers
{

void change_group(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_auth_change_group& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
