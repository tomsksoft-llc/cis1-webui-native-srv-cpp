#include "websocket_router.h"

#include "response.h"

web_app::handle_result websocket_router::handlers_chain::handle(
            tcp::socket& socket,
            web_app::request_t& req,
            web_app::context_t& ctx)
{
    for(auto& handler : handlers_)
    {
        auto result = handler(socket, req, ctx);
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

void websocket_router::handlers_chain::append_handler(const web_app::ws_handler_t& handler)
{
    handlers_.push_back(handler);
}

web_app::handle_result websocket_router::operator()(
            tcp::socket& socket,
            web_app::request_t& req,
            web_app::context_t& ctx)
{
    std::string target{req.target()}; //TODO: use string_view somehow
    for(auto&& [regexp, chain] : routes_)
    {
        boost::smatch what;
        if(boost::regex_match(target, what, regexp))
        {
            chain->handle(socket, req, ctx);
            return web_app::handle_result::done;
        }
    }
    return web_app::handle_result::done;
}

using namespace std::string_literals;

websocket_router::handlers_chain& websocket_router::add_route(const std::string& route)
{
    auto&& inserted = routes_.emplace_back(
            "^"s + route + "$"s,
            std::make_unique<handlers_chain>());
    return *(inserted.second);
}

websocket_router::handlers_chain& websocket_router::add_catch_route()
{
    auto&& inserted = routes_.emplace_back(
            "^/.+$",
            std::make_unique<handlers_chain>());
    return *(inserted.second);
}

