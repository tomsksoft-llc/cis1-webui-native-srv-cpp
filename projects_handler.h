#pragma once

#include "cis_util.h"
#include "web_app.h"
#include "net/queued_websocket_session.h"

class projects_handler
{
    project_list projects_;
public:
    projects_handler();
    void get_project_list(
            const rapidjson::Document& data,
            websocket_queue& queue,
            web_app::context_t& ctx);
    void get_subproject_list(
            const rapidjson::Document& data,
            websocket_queue& queue,
            web_app::context_t& ctx);
    void run(
            boost::asio::io_context& ctx,
            const std::string& project,
            const std::string& job);
    web_app::handle_result update(
            web_app::request_t& req,
            web_app::queue_t& queue,
            web_app::context_t& ctx);
};
