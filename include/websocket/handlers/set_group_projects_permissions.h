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

#include "websocket/dto/group_projects_permissions_set.h"

namespace websocket::handlers
{

void set_group_projects_permissions(
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::group_projects_permissions_set& req,
        cis1::proto_utils::transaction tr);

} // namespace websocket::handlers
