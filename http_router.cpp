#include "http_router.h"

#include "response.h"

web_app::handle_result http_router::handlers_chain::handle(
            web_app::request_t& req,
            web_app::queue_t& queue,
            web_app::context_t& ctx)
{
    for(auto& handler : handlers_)
    {
        auto result = handler(req, queue, ctx);
        switch(result)
        {
            case web_app::handle_result::next:
                break;
            case web_app::handle_result::done:
                return web_app::handle_result::done;
            case web_app::handle_result::error:
                return web_app::handle_result::error;
        };
    }
    return web_app::handle_result::next;
}

void http_router::handlers_chain::append_handler(const web_app::handler_t& handler)
{
    handlers_.push_back(handler);
}

web_app::handle_result http_router::operator()(
            web_app::request_t& req,
            web_app::queue_t& queue,
            web_app::context_t& ctx)
{
    std::string target{req.target()}; //TODO: use string_view somehow
    for(auto&& [regexp, chain] : routes_)
    {
        boost::smatch what;
        if(boost::regex_match(target, what, regexp))
        {
            chain->handle(req, queue, ctx);
            return web_app::handle_result::done;
        }
    }
    queue.send(response::not_found(std::move(req)));
    return web_app::handle_result::done;
}

using namespace std::string_literals;

http_router::handlers_chain& http_router::add_route(const std::string& route)
{
    auto&& inserted = routes_.emplace_back(
            "^"s + route + "$"s,
            std::make_unique<handlers_chain>());
    return *(inserted.second);
}

http_router::handlers_chain& http_router::add_catch_route()
{
    auto&& inserted = routes_.emplace_back(
            "^/.+$",
            std::make_unique<handlers_chain>());
    return *(inserted.second);
}

/*
void router::handle(
        http::request<http::string_body>&& req,
        http_session::queue& queue) const
{
    std::string target = req.target().to_string();
    for(auto&& [regexp, handler] : routes_)
    {
        boost::smatch what;
        if(boost::regex_match(target, what, regexp))
        {
            return handler(std::move(req), queue);
        }
    }
    return queue.send(response::not_found(std::move(req)));
};

void router::handle_upgrade(
        tcp::socket&& socket,
        http::request<http::string_body>&& req) const
{
    std::string target = req.target().to_string();
    for(auto&& [regexp, handler] : ws_routes_)
    {
        boost::smatch what;
        if(boost::regex_match(target, what, regexp))
        {
            return handler(std::move(socket), std::move(req));
        }
    }
};

using namespace std::string_literals;

void router::add_route(std::string route, std::function<void(http::request<http::string_body>&&, http_session::queue&)> handle_cb)
{
    std::string regex = "^"s + route + "$"s;
    routes_.emplace_back(regex, handle_cb);
}

void router::add_catch_route(std::function<void(http::request<http::string_body>&&, http_session::queue&)> handle_cb)
{
    routes_.emplace_back("^/.+$", handle_cb);
}

void router::add_ws_route(std::string route, std::function<void(tcp::socket&&, http::request<http::string_body>&&)> handle_cb)
{
    std::string regex = "^"s + route + "(\\?.+$|$)"s;
    ws_routes_.emplace_back(regex, handle_cb);
}

void router::add_ws_catch_route(std::function<void(tcp::socket&&, http::request<http::string_body>&&)> handle_cb)
{
    ws_routes_.emplace_back("^/.+$", handle_cb);
}
*/
