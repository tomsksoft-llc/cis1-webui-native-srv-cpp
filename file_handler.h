#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

namespace http = boost::beast::http;

class file_handler
{
    const std::string doc_root_;
public:
    file_handler(const std::string& doc_root);
    handle_result operator()(
            http::request<http::empty_body>& req,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            request_context& ctx);

    handle_result single_file(
            http::request<http::empty_body>& req,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            request_context& ctx,
            std::string_view path);
};
