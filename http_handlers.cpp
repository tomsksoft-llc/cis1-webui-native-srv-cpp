#include "http_handlers.h"

handle_result handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        http::request<http::string_body>& /*req*/,
        http_session::queue& /*queue*/,
        request_context& ctx)
{
    if(const auto& cookies = ctx.cookies; cookies.count("token"))
    {
        auto user = authentication_handler->authenticate(cookies.at("token"));
        if(!user.empty())
        {
            ctx.username = user;
        }
    }
    return handle_result::next;
}
