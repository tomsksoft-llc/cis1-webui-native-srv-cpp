#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "auth_manager_interface.h"
#include "cis/cis_manager.h"
#include "transaction.h"

#include "websocket/dto/cis_cron_list.h"

namespace websocket
{

namespace handlers
{

void list_cis_cron(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::cis_cron_list& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
