#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/cis_job_run.h"

namespace websocket
{

namespace handlers
{

void run_job(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::cis_job_run& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
