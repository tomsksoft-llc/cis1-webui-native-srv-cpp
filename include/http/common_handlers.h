/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include "request_context.h"
#include "net/http_session.h"
#include "auth_manager.h"
#include "cis/cis_structs.h"

namespace beast = boost::beast;

namespace http
{

handle_result handle_authenticate(
        auth_manager& authentication_handler,
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue);

} // namespace http
