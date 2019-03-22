#pragma once

#include "request_context.h"
#include "web_app.h"

class file_handler
{
    const std::string doc_root_;
public:
    file_handler(const std::string& doc_root);
    web_app::handle_result operator()(
            web_app::request_t& req,
            web_app::queue_t& queue,
            request_context& ctx);

    web_app::handle_result single_file(
            web_app::request_t& req,
            web_app::queue_t& queue,
            request_context& ctx,
            std::string_view path);
};
