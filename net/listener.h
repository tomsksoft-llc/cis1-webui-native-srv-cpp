#pragma once

#include <memory>

#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::function<void(tcp::socket&&)> accept_socket_;
public:
    listener(
        net::io_context& ioc,
        std::function<void(tcp::socket&&)> accept_socket);

    void listen(const tcp::endpoint& endpoint, beast::error_code& ec);

    // Start accepting incoming connections
    void run();

    void do_accept();

    void on_accept(beast::error_code ec);
};

