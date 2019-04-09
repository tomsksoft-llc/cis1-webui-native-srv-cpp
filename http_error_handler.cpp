#include "http_error_handler.h"

#include "response.h"

void http_error_handler::operator()(
        http::request<http::empty_body>& req,
        request_context& ctx,
        net::http_session::queue& queue)
{
    using status = boost::beast::http::status;

    switch(ctx.res_status)
    {
        case status::not_found:
        {
            queue.send(response::not_found(std::move(req)));
            break;
        }
        default:
        {
            queue.send(response::server_error(std::move(req), ctx.error));
        }
    }
}
