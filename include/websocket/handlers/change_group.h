#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "auth_manager_interface.h"
#include "transaction.h"

#include "websocket/dto/user_auth_change_group.h"

namespace websocket
{

namespace handlers
{

void change_group(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_auth_change_group& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
