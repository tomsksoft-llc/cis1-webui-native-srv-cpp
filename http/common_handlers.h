#pragma once

#include "request_context.h"
#include "net/http_session.h"
#include "auth_manager.h"
#include "cis/cis_structs.h"

namespace beast = boost::beast;

namespace http
{

handle_result handle_authenticate(
        auth_manager& authentication_handler,
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue);

handle_result handle_update_projects(
        cis::project_list& projects,
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue);

} // namespace http
