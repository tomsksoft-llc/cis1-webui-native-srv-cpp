#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "rights_manager_interface.h"
#include "cis/cis_manager.h"

#include "websocket/dto/fs_entry_info.h"

namespace websocket
{

namespace handlers
{

void get_fs_entry_info(
        cis::cis_manager_interface& cis_manager,
        rights_manager_interface& rights,
        request_context& ctx,
        const dto::fs_entry_info& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
