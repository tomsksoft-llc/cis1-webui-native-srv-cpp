#include "common_handlers.h"

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

handle_result handle_update_projects(
        const std::shared_ptr<cis::project_list>& projects,
        beast::http::request<beast::http::empty_body>& req,
        request_context& /*ctx*/,
        net::http_session::request_reader& /*reader*/,
        net::http_session::queue& queue)
{
    projects->fetch();

    beast::http::response<beast::http::empty_body> res{
        beast::http::status::ok,
        req.version()};
    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "application/json");
    res.prepare_payload();
    res.keep_alive(req.keep_alive());

    queue.send(std::move(res));
    return handle_result::done;
}

} // namespace http
