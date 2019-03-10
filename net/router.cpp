#include "router.h"

#include "response.h"

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
