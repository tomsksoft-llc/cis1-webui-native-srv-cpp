/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "rights_manager_interface.h"

#include "websocket/dto/admin_user_permission_set.h"

namespace websocket
{

namespace handlers
{

void set_user_permissions(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_user_permission_set& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
