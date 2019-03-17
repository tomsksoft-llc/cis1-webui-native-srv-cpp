#pragma once

#include <functional>
#include <vector>

#include <boost/regex.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "web_app.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

class websocket_router
{
public:
    class handlers_chain
    {
        std::vector<web_app::ws_handler_t> handlers_;
    public:
        web_app::handle_result handle(
                tcp::socket& socket,
                web_app::request_t& req,
                web_app::context_t& ctx);
        void append_handler(web_app::ws_handler_t handler);
    };
    web_app::handle_result operator()(
            tcp::socket& socket,
            web_app::request_t& req,
            web_app::context_t& ctx);
    handlers_chain& add_route(std::string route);
    handlers_chain& add_catch_route();
private:
    std::vector<std::pair<boost::regex, std::unique_ptr<handlers_chain>>> routes_;
};
