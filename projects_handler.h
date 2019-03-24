#pragma once

#include "cis_util.h"
#include "handle_result.h"
#include "request_context.h"
#include "net/queued_websocket_session.h"
#include "net/http_session.h"

class projects_handler
{
    project_list projects_;
public:
    projects_handler();
    void get_project_list(
            const rapidjson::Document& data,
            websocket_queue& queue,
            request_context& ctx);
    void run(
            boost::asio::io_context& ctx,
            const std::string& project,
            const std::string& job);
    handle_result update(
            http::request<http::string_body>& req,
            http_session::queue& queue,
            request_context& ctx);
};
