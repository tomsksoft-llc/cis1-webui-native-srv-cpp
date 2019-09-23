#include "error_handler.h"

#include "response.h"

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
            queue.send(response::not_found(std::move(req)));
            break;
        }
        case status::forbidden:
        {
            queue.send(response::forbidden(std::move(req)));
            break;
        }
        default:
        {
            queue.send(response::server_error(std::move(req), ctx.error));
        }
    }
}

} // namespace http
