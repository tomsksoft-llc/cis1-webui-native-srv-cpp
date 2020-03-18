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
#include "auth_manager_interface.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager_interface.h"

#include "websocket/dto/admin_user_set_admin_status.h"

namespace websocket::handlers
{

void set_admin_status(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_user_set_admin_status& req,
        cis1::proto_utils::transaction tr);

} // namespace websocket::handlers
