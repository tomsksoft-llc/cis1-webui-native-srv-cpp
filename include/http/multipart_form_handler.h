#pragma once

#include <filesystem>

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"
#include "rights_manager.h"
#include "beast_ext/multipart_form_body.h"

namespace beast = boost::beast;

namespace http
{

//TODO write full implementation
class multipart_form_handler
{
public:
    multipart_form_handler(
            std::filesystem::path files_root,
            rights_manager& rights);

    handle_result upload(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& dir = "");

    handle_result replace(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& dir = "");

private:
    std::filesystem::path files_root_;
    rights_manager& rights_;

    void handle_body(
            beast::http::request<multipart_form_body>&& req,
            request_context& ctx,
            const boost::beast::error_code& ec,
            net::http_session::queue& queue);
};

} // namespace http
