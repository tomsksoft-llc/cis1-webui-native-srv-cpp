#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "auth_manager.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/cis_cron_list.h"

namespace websocket
{

namespace handlers
{

void list_cis_cron(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_cron_list& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
