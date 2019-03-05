#pragma once

#include "http_session.h"

class projects_handler
{
    const std::string cis_jobs_;
public:
    projects_handler(const std::string& cis_root);
    void handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue);
};
