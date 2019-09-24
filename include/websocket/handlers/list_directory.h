#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager.h"
#include "websocket/transaction.h"

#include "websocket/dto/fs_entry_list.h"

namespace websocket
{

namespace handlers
{

void list_directory(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_list& req,
        transaction tr);

} // namespace handlers

} // namespace websocket