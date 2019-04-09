#include "http_handlers.h"

#include "response.h"

handle_result handle_authenticate(
        const std::shared_ptr<auth_manager>& authentication_handler,
        http::request<http::empty_body>& /*req*/,
        net::http_session::request_reader& reader,
        net::http_session::queue& /*queue*/,
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

handle_result handle_update_projects(
        const std::shared_ptr<project_list>& projects,
        http::request<http::empty_body>& req,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        request_context& /*ctx*/)
{
    beast::error_code ec;
    // Handle an unknown error
    if(ec)
    {
        queue.send(response::server_error(std::move(req), ec.message()));
        return handle_result::done;
    }

    projects->fetch();

    http::response<http::empty_body> res{
        http::status::ok,
        req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/json");
    res.prepare_payload();
    res.keep_alive(req.keep_alive());
    queue.send(std::move(res));
    return handle_result::done;
}
