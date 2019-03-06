#pragma once

#include "http_session.h"
#include "cis_util.h"

class projects_handler
{
    job_list jobs_;
public:
    projects_handler(const std::string& cis_root);
    void handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue);
    void update();
private:
    void on_jobs_update();
};
