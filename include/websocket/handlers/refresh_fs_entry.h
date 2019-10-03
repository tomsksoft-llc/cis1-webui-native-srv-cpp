#pragma once

#include "request_context.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager.h"
#include "transaction.h"

#include "websocket/dto/fs_entry_refresh.h"

namespace websocket
{

namespace handlers
{

void refresh_fs_entry(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_refresh& req,
        transaction tr);

} // namespace handlers

} // namespace websocket
