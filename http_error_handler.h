#pragma once

#include "net/http_session.h"
#include "request_context.h"

namespace http = boost::beast::http;

class http_error_handler
{
public:
    void operator()(
            http::request<http::empty_body>& req,
            request_context& ctx,
            net::http_session::queue& queue);
};
