#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/fs_entry_remove.h"

namespace websocket
{

namespace handlers
{

void remove_fs_entry(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_remove& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
