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
#include "url.h"
#include "meta.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace asio = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

namespace http
{

template <typename... Args>
class router
{
public:
    using request_t = beast::http::request<beast::http::empty_body>;
    using context_t = request_context;
    using handler_t = std::function<handle_result(request_t&, context_t&, Args...)>;
    struct handler_interface
    {
        virtual ~handler_interface() = default;
        virtual handle_result handle(
                request_t&,
                context_t&,
                Args...,
                const boost::smatch&) const = 0;
    };
    handle_result operator()(request_t& req, context_t& ctx, Args... args)
    {
        std::string target{req.target()}; //TODO: use string_view somehow
        for(auto&& [regexp, handler] : routes_)
        {
            boost::smatch what;
            if(boost::regex_match(target, what, regexp))
            {
                return handler->handle(req, ctx, std::forward<decltype(args)>(args)..., what);
            }
        }

        return handle_result::error;
    }
    template <
        class ParseString,
        class... RouteArgs,
        class Fn>
    void add_route(
            url::url_chain<ParseString, RouteArgs...> url,
            Fn cb)
    {
        using namespace std::string_literals;
        class handler_impl
            : public handler_interface
        {
            Fn fn;
        public:
            handler_impl(
                    Fn fn_arg)
                : fn(fn_arg)
            {}

            handle_result handle(
                    request_t& req,
                    context_t& ctx,
                    Args... args,
                    const boost::smatch& what) const
            {
                auto parsed_args = meta::maybe_tuple<RouteArgs...>(what, 1);
                if(parsed_args)
                {
                    auto fn_args = std::tuple_cat(
                            std::forward_as_tuple(req, ctx, args...),
                            parsed_args.value());
                    return std::apply(fn, fn_args);
                }
                return handle_result::error;
            }
        };

        auto&& inserted = routes_.emplace_back(
                "^"s + ParseString::value + "$"s,
                std::make_unique<handler_impl>(cb));
    }
private:
    std::vector<std::pair<boost::regex, std::unique_ptr<handler_interface>>> routes_;
};

} // namespace http

using http_router = http::router<
        net::http_session::request_reader&,
        net::http_session::queue&>;
using websocket_router = http::router<tcp::socket&>;
