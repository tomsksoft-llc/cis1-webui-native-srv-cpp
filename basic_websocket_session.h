#pragma once

#include <memory>
#include <chrono>
#include <functional>

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

class basic_websocket_session
    : public std::enable_shared_from_this<basic_websocket_session>
{
protected:
    websocket::stream<tcp::socket> ws_;
    net::strand<
        net::io_context::executor_type> strand_;
    net::steady_timer timer_;
    char ping_state_ = 0;
public:
    // Take ownership of the socket
    explicit basic_websocket_session(tcp::socket socket);

    // Start the asynchronous operation
    template<class Body, class Allocator>
    void do_accept(http::request<Body, http::basic_fields<Allocator>> req);

    void on_accept(beast::error_code ec);

    // Called when the timer expires.
    void on_timer(beast::error_code ec);

    // Called to indicate activity from the remote peer
    void activity();

    // Called after a ping is sent.
    void on_ping(beast::error_code ec);

    void on_control_callback(
        websocket::frame_type kind,
        beast::string_view payload);

    virtual void on_accept_success() = 0;
protected:
    template <typename Derived>
    std::shared_ptr<Derived> shared_from_base()
    {
        return std::static_pointer_cast<Derived>(shared_from_this());
    }
};

template<class Body, class Allocator>
void basic_websocket_session::do_accept(http::request<Body, http::basic_fields<Allocator>> req)
{
    // Set the control callback. This will be called
    // on every incoming ping, pong, and close frame.
    ws_.control_callback(
        std::bind(
            &basic_websocket_session::on_control_callback,
            this,
            std::placeholders::_1,
            std::placeholders::_2));
    
    // Run the timer. The timer is operated
    // continuously, this simplifies the code.
    on_timer({});

    // Set the timer
    timer_.expires_after(std::chrono::seconds(15));
    
    // Accept the websocket handshake
    ws_.async_accept(
        req,
        net::bind_executor(
            strand_,
            std::bind(
                &basic_websocket_session::on_accept,
                shared_from_this(),
                std::placeholders::_1)));
}
