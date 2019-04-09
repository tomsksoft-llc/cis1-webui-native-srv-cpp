#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

namespace beast = boost::beast;

namespace http
{

class file_handler
{
    const std::string doc_root_;
public:
    file_handler(const std::string& doc_root);
    handle_result operator()(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue);

    handle_result single_file(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            std::string_view path);
};

} // namespace http
