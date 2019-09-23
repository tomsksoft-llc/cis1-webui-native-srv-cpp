#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "auth_manager_interface.h"
#include "websocket/transaction.h"

#include "websocket/dto/user_list.h"

namespace websocket
{

namespace handlers
{

void list_users(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_list& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
