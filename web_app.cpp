#include "web_app.h"

#include "net/listener.h"

web_app::web_app(boost::asio::io_context& ioc)
    : ioc_(ioc)
{}

void web_app::append_handler(const handler_t& handler)
{
    handlers_.push_back(handler);
}

void web_app::append_ws_handler(const ws_handler_t& handler)
{
    ws_handlers_.push_back(handler);
}

void web_app::set_error_handler(const handler_t& handler)
{
    error_handler_ = handler;
}

void web_app::set_ws_error_handler(const ws_handler_t& handler)
{
    ws_error_handler_ = handler;
}

void web_app::listen(const tcp::endpoint& endpoint)
{
    auto accept_handler = 
    [self = shared_from_this()](tcp::socket&& socket){
        std::make_shared<http_session>(
            std::move(socket),
            self)->run();
    };
    auto l = std::make_shared<listener>(
        ioc_,
        accept_handler);
    beast::error_code ec;
    l->listen(endpoint, ec);
    if(ec)
    {
        throw "failure"; //TODO
    }
    l->run();
}

void web_app::handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue) const
{
    context_t ctx{};
    for(auto& handler : handlers_)
    {
        auto result = handler(req, queue, ctx);
        switch(result)
        {
            case handle_result::next:
                break;
            case handle_result::done:
                return;
            case handle_result::error:
                error_handler_(req, queue, ctx);
                return;
        };
    }
}

void web_app::handle_upgrade(
        tcp::socket&& socket,
        http::request<http::string_body>&& req) const
{
    context_t ctx{};
    for(auto& handler : ws_handlers_)
    {
        auto result = handler(req, socket, ctx);
        switch(result)
        {
            case handle_result::next:
                break;
            case handle_result::done:
                return;
            case handle_result::error:
                ws_error_handler_(req, socket, ctx);
                return;
        };
    }
};
