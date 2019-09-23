#include "listener.h"

#include <functional>

#include "fail.h"
#include "socket_util.h"

namespace net
{

listener::listener(
    boost::asio::io_context& ioc,
    std::function<void(boost::asio::ip::tcp::socket&&)> accept_socket)
    : acceptor_(ioc)
    , socket_(ioc)
    , accept_socket_(std::move(accept_socket))
{}

void listener::listen(
        const boost::asio::ip::tcp::endpoint& endpoint,
        boost::beast::error_code& ec)
{

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "open");
        return;
    }

    // Automaticly close socket on exec
    set_cloexec(acceptor_, ec);
    if(ec)
    {
        fail(ec, "set_cloexec");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if(ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
            boost::asio::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }
}

void listener::run()
{
    if(!acceptor_.is_open())
    {
        return;
    }
    do_accept();
}

void listener::do_accept()
{
    acceptor_.async_accept(
            socket_,
            std::bind(
                    &listener::on_accept,
                    shared_from_this(),
                    std::placeholders::_1));
}

void listener::on_accept(boost::beast::error_code ec)
{
    if(ec)
    {
        fail(ec, "accept");
    }
    else
    {
        // Automaticly close socket on exec
        set_cloexec(socket_, ec);
        if(ec)
        {
            fail(ec, "set_cloexec");
            return;
        }
        // Create the http_session and run it
        accept_socket_(std::move(socket_));
    }

    // Accept another connection
    do_accept();
}

} // namespace net
