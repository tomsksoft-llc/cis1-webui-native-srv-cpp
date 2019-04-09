#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

namespace http = boost::beast::http;

class cookie_parser
{
public:
    static handle_result parse(
            http::request<http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue);
};
