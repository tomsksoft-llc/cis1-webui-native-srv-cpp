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
    if(const auto& cookies = ctx.cookies; cookies.count("token"))
    {
        auto username = authentication_handler.authenticate(cookies.at("token"));
        if(username)
        {
            ctx.username = username.value();
        }
    }
    return handle_result::next;
}

} // namespace http
