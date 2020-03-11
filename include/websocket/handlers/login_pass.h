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

#include "websocket/dto/auth_login_pass.h"

namespace websocket
{

namespace handlers
{

void login_pass(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::auth_login_pass& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
