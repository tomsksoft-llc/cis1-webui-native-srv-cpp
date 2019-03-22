#pragma once

#include "cis_util.h"
#include "request_context.h"
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
            request_context& ctx);
    void get_subproject_list(
            const rapidjson::Document& data,
            websocket_queue& queue,
            request_context& ctx);
    void run(
            boost::asio::io_context& ctx,
            const std::string& project,
            const std::string& job);
    web_app::handle_result update(
            web_app::request_t& req,
            web_app::queue_t& queue,
            request_context& ctx);
};
