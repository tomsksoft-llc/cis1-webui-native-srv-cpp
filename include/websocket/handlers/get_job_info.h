#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/cis_job_info.h"

namespace websocket
{

namespace handlers
{

void get_job_info(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_job_info& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
