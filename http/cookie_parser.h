#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

namespace beast = boost::beast;

namespace http
{

class cookie_parser
{
public:
    static handle_result parse(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue);
};

} // namespace http
