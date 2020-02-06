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

#include "websocket/dto/user_permissions_projects_get.h"

namespace websocket
{

namespace handlers
{

void get_user_permissions_projects(
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::user_permissions_projects_get& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
