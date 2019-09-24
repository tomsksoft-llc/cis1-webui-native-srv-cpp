#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/cis_project_info.h"

namespace websocket
{

namespace handlers
{

void get_project_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_project_info& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
