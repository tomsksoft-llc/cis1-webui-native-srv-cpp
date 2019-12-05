/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <memory>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace net
{

class listener
    : public std::enable_shared_from_this<listener>
{
public:
    listener(
            boost::asio::io_context& ioc,
            std::function<void(boost::asio::ip::tcp::socket&&)> accept_socket);

    void listen(
            const boost::asio::ip::tcp::endpoint& ep,
            boost::beast::error_code& ec);

    // Start accepting incoming connections
    void run();

    void do_accept();

    void on_accept(boost::beast::error_code ec);

private:
    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    std::function<void(boost::asio::ip::tcp::socket&&)> accept_socket_;
};

} // namespace net
