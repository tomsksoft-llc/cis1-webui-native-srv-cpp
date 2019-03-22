#pragma once

#include <functional>
#include <vector>

#include <boost/regex.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"
#include "response.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

template <typename... Args>
class router
{
public:
    using request_t = http::request<http::string_body>;
    using handler_t = std::function<handle_result(request_t& req, Args...)>;
    class handlers_chain
    {
        std::vector<handler_t> handlers_;
    public:
        handle_result handle(request_t& req, Args... args)
        {
            for(auto& handler : handlers_)
            {
                auto result = handler(req, std::forward<decltype(args)>(args)...);
                switch(result)
                {
                    case handle_result::next:
                        break;
                    case handle_result::done:
                        return handle_result::done;
                    case handle_result::error:
                        return handle_result::error;
                };
            }
            return handle_result::next;
        }
        void append_handler(const handler_t& handler)
        {
            handlers_.push_back(handler);
        }
    };
    handle_result operator()(request_t& req, Args... args)
    {
        std::string target{req.target()}; //TODO: use string_view somehow
        for(auto&& [regexp, chain] : routes_)
        {
            boost::smatch what;
            if(boost::regex_match(target, what, regexp))
            {
                chain->handle(req, std::forward<decltype(args)>(args)...);
                return handle_result::done;
            }
        }
        //queue.send(response::not_found(std::move(req)));
        return handle_result::done;
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

using http_router = router<http_session::queue&, request_context&>;
using websocket_router = router<tcp::socket&, request_context&>;
