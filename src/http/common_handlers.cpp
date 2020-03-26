/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "http/common_handlers.h"

namespace http
{

handle_result handle_authenticate(
        auth_manager& authentication_handler,
        beast::http::request<beast::http::empty_body>& /*req*/,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& /*queue*/)
{
    if(const auto& cookies = ctx.cookies; cookies.count("auth_token"))
    {
        std::error_code ec;

        auto email = authentication_handler.authenticate(cookies.at("auth_token"), ec);

        if(ec)
        {
            ctx.res_status = boost::beast::http::status::internal_server_error;

            return handle_result::error;
        }

        if(email)
        {
            ctx.client_info = request_context::user_info{email.value()};
        }
    }
    return handle_result::next;
}

} // namespace http
