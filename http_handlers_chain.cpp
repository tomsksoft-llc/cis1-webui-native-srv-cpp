#include "http_handlers_chain.h"

#include "net/listener.h"
#include "exceptions/generic_error.h"

http_handlers_chain::http_handlers_chain()
{}

void http_handlers_chain::append_handler(const handler_t& handler)
{
    handlers_.push_back(handler);
}

void http_handlers_chain::append_ws_handler(const ws_handler_t& handler)
{
    ws_handlers_.push_back(handler);
}

void http_handlers_chain::set_error_handler(const error_handler_t& handler)
{
    error_handler_ = handler;
}

void http_handlers_chain::listen(boost::asio::io_context& ioc, const tcp::endpoint& endpoint)
{
    auto accept_handler = 
    [self = shared_from_this()](tcp::socket&& socket){
        std::make_shared<net::http_session>(
            std::move(socket),
            self)->run();
    };
    auto l = std::make_shared<net::listener>(
        ioc,
        accept_handler);
    boost::beast::error_code ec;
    l->listen(endpoint, ec);
    if(ec)
    {
        throw generic_error(ec.message());
    }
    l->run();
}

void http_handlers_chain::handle_header(
            request_header_t& req,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue) const
{
    context_t ctx{};
    for(auto& handler : handlers_)
    {
        auto result = handler(req, ctx, reader, queue);
        switch(result)
        {
            case handle_result::next:
                break;
            case handle_result::done:
                return;
            case handle_result::error:
                if(error_handler_)
                {
                    error_handler_(req, ctx, queue);
                }
                return;
        };
    }
}

void http_handlers_chain::handle_upgrade(
        tcp::socket&& socket,
        request_header_t&& req,
        net::http_session::queue& queue) const
{
    context_t ctx{};
    for(auto& handler : ws_handlers_)
    {
        auto result = handler(req, ctx, socket);
        switch(result)
        {
            case handle_result::next:
                break;
            case handle_result::done:
                return;
            case handle_result::error:
                if(error_handler_)
                {
                    error_handler_(req, ctx, queue);
                }
                return;
        };
    }
};
