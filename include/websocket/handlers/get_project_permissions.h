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
#include "auth_manager_interface.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager_interface.h"

#include "websocket/dto/admin_project_permission_get.h"

namespace websocket::handlers
{

void get_project_permissions(
        cis::cis_manager_interface& cis_manager,
        auth_manager_interface& authentication_handler,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::admin_project_permission_get& req,
        cis1::proto_utils::transaction tr);

} // namespace websocket::handlers
