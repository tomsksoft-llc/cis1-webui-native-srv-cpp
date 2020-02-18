/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "rights_manager_interface.h"
#include "auth_manager_interface.h"

#include "websocket/dto/user_auth_add.h"

namespace websocket::handlers
{

void add_user(auth_manager_interface& authentication_handler,
              rights_manager_interface& rights,
              request_context& ctx,
              const dto::user_auth_add& req,
              cis1::proto_utils::transaction tr);

} // namespace websocket::handlers
