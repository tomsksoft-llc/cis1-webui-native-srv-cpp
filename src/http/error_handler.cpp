/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "http/error_handler.h"

#include "http/response.h"
#include "logger.h"

namespace http
{

void error_handler::operator()(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::queue& queue)
{
    using status = beast::http::status;

    switch(ctx.res_status)
    {
        case status::not_found:
        {
            LOG(scl::Level::Info, "%s not found", req.target());
            queue.send(response::not_found(std::move(req)));
            break;
        }
        case status::forbidden:
        {
            LOG(scl::Level::Info, "%s is forbidden", req.target());
            queue.send(response::forbidden(std::move(req)));
            break;
        }
        default:
        {
            LOG(scl::Level::Info, R"(Server error on handle "%s": %s)", req.target(), ctx.error);
            queue.send(response::server_error(std::move(req), ctx.error));
        }
    }
}

} // namespace http
