#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_permissions_get.h"

namespace websocket
{

namespace handlers
{

void get_user_permissions(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_permissions_get& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
