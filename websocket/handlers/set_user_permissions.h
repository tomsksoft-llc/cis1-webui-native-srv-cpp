#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_permissions_set.h"

namespace websocket
{

namespace handlers
{

void set_user_permissions(
        rights_manager& rights,
        request_context& ctx,
        const dto::user_permissions_set& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
