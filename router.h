#pragma once

#include <functional>
#include <vector>

#include <boost/regex.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "web_app.h"
#include "response.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

template <typename... Args>
class router
{
public:
    using handler_t = std::function<web_app::handle_result(web_app::request_t& req, Args...)>;
    class handlers_chain
    {
        std::vector<handler_t> handlers_;
    public:
        web_app::handle_result handle(web_app::request_t& req, Args... args)
        {
            for(auto& handler : handlers_)
            {
                auto result = handler(req, std::forward<decltype(args)>(args)...);
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
        void append_handler(const handler_t& handler)
        {
            handlers_.push_back(handler);
        }
    };
    web_app::handle_result operator()(web_app::request_t& req, Args... args)
    {
        std::string target{req.target()}; //TODO: use string_view somehow
        for(auto&& [regexp, chain] : routes_)
        {
            boost::smatch what;
            if(boost::regex_match(target, what, regexp))
            {
                chain->handle(req, std::forward<decltype(args)>(args)...);
                return web_app::handle_result::done;
            }
        }
        //queue.send(response::not_found(std::move(req)));
        return web_app::handle_result::done;
    };
    handlers_chain& add_route(const std::string& route)
    {
        using namespace std::string_literals;
        auto&& inserted = routes_.emplace_back(
                "^"s + route + "$"s,
                std::make_unique<handlers_chain>());
        return *(inserted.second);
    }
    handlers_chain& add_catch_route()
    {
        auto&& inserted = routes_.emplace_back(
                "^/.+$",
                std::make_unique<handlers_chain>());
        return *(inserted.second);
    }
private:
    std::vector<std::pair<boost::regex, std::unique_ptr<handlers_chain>>> routes_;
};

using http_router = router<web_app::queue_t&, web_app::context_t&>;
using websocket_router = router<tcp::socket&, web_app::context_t&>;
