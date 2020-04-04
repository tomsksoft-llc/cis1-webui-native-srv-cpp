/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "http/handlers_chain.h"

#include "net/listener.h"
#include "exceptions/generic_error.h"
#include "error_code.h"

namespace http
{

handlers_chain::handlers_chain(boost::asio::io_context& ioc)
    : ioc_(ioc)
{}

void handlers_chain::append_handler(const handler_t& handler)
{
    handlers_.push_back(handler);
}

void handlers_chain::append_ws_handler(const ws_handler_t& handler)
{
    ws_handlers_.push_back(handler);
}

void handlers_chain::set_error_handler(const error_handler_t& handler)
{
    error_handler_ = handler;
}

void handlers_chain::listen(
        const boost::asio::ip::tcp::endpoint& ep,
        std::error_code& ec)
{
    auto accept_handler = [self = shared_from_this()](tcp::socket&& socket)
    {
        std::make_shared<net::http_session>(
                std::move(socket),
                self)->run();
    };

    auto l = std::make_shared<net::listener>(
            ioc_,
            accept_handler);

    boost::beast::error_code boost_ec;

    l->listen(ep, boost_ec);

    if(boost_ec)
    {
        ec = cis::error_code::cant_run_http_listener;

        return;
    }

    l->run();
}

void handlers_chain::handle_header(
            const boost::asio::ip::tcp::socket& socket,
            request_header_t& req,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue) const
{
    context_t ctx{};
    ctx.remote_addr = std::make_pair(socket.remote_endpoint().address().to_string(),
                                     socket.remote_endpoint().port());

    for(auto& handler : handlers_)
    {
        auto result = handler(req, ctx, reader, queue);
        switch(result)
        {
            case handle_result::next:
                break;
            case handle_result::done:
                reader.done();
                return;
            case handle_result::error:
                if(error_handler_)
                {
                    error_handler_(req, ctx, queue);
                }
                reader.done();
                return;
        };
    }

    if(error_handler_)
    {
        error_handler_(req, ctx, queue);
    }
}

void handlers_chain::handle_upgrade(
        tcp::socket&& socket,
        request_header_t&& req,
        net::http_session::queue& queue) const
{
    context_t ctx{};
    ctx.remote_addr = std::make_pair(socket.remote_endpoint().address().to_string(),
                                     socket.remote_endpoint().port());

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

    if(error_handler_)
    {
        error_handler_(req, ctx, queue);
    }
};

} // namespace http
