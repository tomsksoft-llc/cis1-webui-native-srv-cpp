#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

namespace http = boost::beast::http;

//TODO write full implementation
class multipart_form_handler
{
public:
    handle_result operator()(
            http::request<http::empty_body>& req,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            request_context& ctx,
            const std::string& save_dir);
};
