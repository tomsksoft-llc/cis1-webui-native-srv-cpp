#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "configuration_manager.h"

#include "websocket/dto/system_version_info.h"

namespace websocket
{

namespace handlers
{

void get_system_version(
        configuration_manager& config,
        request_context& ctx,
        const dto::system_version_info& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
