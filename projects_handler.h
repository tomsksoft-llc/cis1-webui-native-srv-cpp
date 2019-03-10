#pragma once

#include "http_session.h"
#include "cis_util.h"

class projects_handler
{
    project_list projects_;
public:
    projects_handler();
    void get_projects(
            http::request<http::string_body>&& req,
            http_session::queue& queue);
    void run(
            boost::asio::io_context& ctx,
            const std::string& project,
            const std::string& job);
    void update();
};
