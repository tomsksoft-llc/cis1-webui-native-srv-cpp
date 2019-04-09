#pragma once

#include "request_context.h"
#include "net/http_session.h"
#include "auth_manager.h"
#include "cis/project_list.h"

namespace http = boost::beast::http;

handle_result handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        http::request<http::empty_body>& req,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        request_context& ctx);

handle_result handle_update_projects(
        const std::shared_ptr<cis::project_list>& projects,
        http::request<http::empty_body>& req,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        request_context& ctx);
