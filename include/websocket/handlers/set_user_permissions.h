#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "transaction.h"

#include "websocket/dto/user_permissions_set.h"

namespace websocket
{

namespace handlers
{

void set_user_permissions(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_permissions_set& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
