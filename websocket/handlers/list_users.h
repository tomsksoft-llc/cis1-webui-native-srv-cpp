#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "auth_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_list.h"

namespace websocket
{

namespace handlers
{

void list_users(
        auth_manager& authentication_handler,
        rights_manager& rights,
        request_context& ctx,
        const dto::user_list& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
