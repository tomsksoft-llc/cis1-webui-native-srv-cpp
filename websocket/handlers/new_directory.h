#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/fs_entry_new_dir.h"

namespace websocket
{

namespace handlers
{

void new_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_new_dir& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
