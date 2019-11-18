/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <cis1_proto_utils/transaction.h>

#include "request_context.h"
#include "cis/cis_manager.h"

#include "websocket/dto/cis_session_unsubscribe.h"

namespace websocket
{

namespace handlers
{

void session_unsubscribe(
        cis::cis_manager_interface& cis_manager,
        request_context& ctx,
        const dto::cis_session_unsubscribe& req,
        cis1::proto_utils::transaction tr);

} // namespace handlers

} // namespace websocket
