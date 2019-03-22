#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

class cookie_parser
{
public:
    static handle_result parse(
            http::request<http::string_body>& req,
            http_session::queue& queue,
            request_context& ctx);
};
