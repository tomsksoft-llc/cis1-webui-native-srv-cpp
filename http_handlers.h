#pragma once

#include "request_context.h"
#include "net/http_session.h"
#include "auth_manager.h"
#include "cis_util.h"

handle_result handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        http::request<http::string_body>& req,
        http_session::queue& queue,
        request_context& ctx);

handle_result handle_update_projects(
        const std::shared_ptr<project_list>& projects,
        http::request<http::string_body>& req,
        http_session::queue& queue,
        request_context& ctx);
