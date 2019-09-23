#pragma once

#include "request_context.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/fs_entry_list.h"

namespace websocket
{

namespace handlers
{

void list_directory(
        cis::cis_manager& cis_manager,
        rights_manager& rights,
        request_context& ctx,
        const dto::fs_entry_list& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
