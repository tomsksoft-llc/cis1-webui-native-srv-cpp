#pragma once

#include "cis_util.h"
#include "web_app.h"

class projects_handler
{
    project_list projects_;
public:
    projects_handler();
    web_app::handle_result operator()(
            web_app::request_t& req,
            web_app::queue_t& queue,
            web_app::context_t& ctx);
    void run(
            boost::asio::io_context& ctx,
            const std::string& project,
            const std::string& job);
    void update();
};
