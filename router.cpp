#include "router.h"

http::response<http::string_body> handlers::accepted(http::request<http::string_body>&& req)
{
    http::response<http::string_body> res{http::status::accepted, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "Request accepted.";
    res.prepare_payload();
    return res;
}

http::response<http::string_body> handlers::not_found(http::request<http::string_body>&& req)
{
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(req.target()) + "' was not found.";
    res.prepare_payload();
    return res;
}

http::response<http::string_body> handlers::bad_request(http::request<http::string_body>&& req, beast::string_view why)
{
    http::response<http::string_body> res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
}

http::response<http::string_body> handlers::server_error(http::request<http::string_body>&& req, beast::string_view what)
{
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
}

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
    return queue.send(handlers::not_found(std::move(req)));
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
    const boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
    const std::string rep("\\\\&");
    std::string result = boost::regex_replace(route, esc, rep,
                                   boost::match_default | boost::format_sed);
    std::string regex = "^"s + result + "$"s;
    routes_.emplace_back(regex, handle_cb);
}

void router::add_catch_route(std::function<void(http::request<http::string_body>&&, http_session::queue&)> handle_cb)
{
    routes_.emplace_back("/.+", handle_cb);
}

void router::add_ws_route(std::string route, std::function<void(tcp::socket&&, http::request<http::string_body>&&)> handle_cb)
{
    const boost::regex esc("[.^$|()\\[\\]{}*+?\\\\]");
    const std::string rep("\\\\&");
    std::string result = boost::regex_replace(route, esc, rep,
                                   boost::match_default | boost::format_sed);
    std::string regex = "^"s + result + "(\\?.+$|$)"s;
    ws_routes_.emplace_back(regex, handle_cb);
}

void router::add_ws_catch_route(std::function<void(tcp::socket&&, http::request<http::string_body>&&)> handle_cb)
{
    ws_routes_.emplace_back("/.+", handle_cb);
}
