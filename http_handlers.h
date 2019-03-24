#pragma once

#include "request_context.h"
#include "auth_manager.h"
#include "net/http_session.h"

handle_result handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        http::request<http::string_body>& req,
        http_session::queue& queue,
        request_context& ctx);
