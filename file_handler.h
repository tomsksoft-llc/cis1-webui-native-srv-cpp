#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

class file_handler
{
    const std::string doc_root_;
public:
    file_handler(const std::string& doc_root);
    handle_result operator()(
            http::request<http::string_body>& req,
            http_session::queue& queue,
            request_context& ctx);

    handle_result single_file(
            http::request<http::string_body>& req,
            http_session::queue& queue,
            request_context& ctx,
            std::string_view path);
};
