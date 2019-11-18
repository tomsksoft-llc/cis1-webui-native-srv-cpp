/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include "net/http_session.h"
#include "request_context.h"

namespace beast = boost::beast;

namespace http
{

class error_handler
{
public:
    void operator()(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::queue& queue);
};

} // namespace http
