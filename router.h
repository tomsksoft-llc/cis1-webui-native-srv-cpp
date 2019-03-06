#pragma once

#include <map>
#include <functional>

#include <boost/regex.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

#include "http_session.h"

namespace handlers
{

http::response<http::string_body> accepted(http::request<http::string_body>&& req);

http::response<http::string_body> not_found(http::request<http::string_body>&& req);

http::response<http::string_body> bad_request(http::request<http::string_body>&& req, beast::string_view why);

http::response<http::string_body> server_error(http::request<http::string_body>&& req, beast::string_view what);

} //namespace handlers

class router
{
public:
    void handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue) const;
    void handle_upgrade(
            tcp::socket&& socket,
            http::request<http::string_body>&& req) const;
    void add_route(std::string route, std::function<void(http::request<http::string_body>&&, http_session::queue&)> handle_cb);
    void add_catch_route(std::function<void(http::request<http::string_body>&&, http_session::queue&)> handle_cb);
    void add_ws_route(std::string route, std::function<void(tcp::socket&&, http::request<http::string_body>&&)> handle_cb);
    void add_ws_catch_route(std::function<void(tcp::socket&&, http::request<http::string_body>&&)> handle_cb);
private:
    std::vector<std::pair<boost::regex, std::function<void(http::request<http::string_body>&&, http_session::queue&)>>> routes_;
    std::vector<std::pair<boost::regex, std::function<void(tcp::socket&&, http::request<http::string_body>&&)>>> ws_routes_;
};
