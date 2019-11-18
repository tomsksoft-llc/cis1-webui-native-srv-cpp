/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

namespace beast = boost::beast;

namespace http
{

class file_handler
{
public:
    file_handler(std::string doc_root, bool ignore_mime = false);

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

    handle_result sef(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue);
private:
    const std::string doc_root_;
    const bool ignore_mime_;
};

} // namespace http
