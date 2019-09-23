#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/cis_project_list_get.h"

namespace websocket
{

namespace handlers
{

void list_projects(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_project_list_get& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
