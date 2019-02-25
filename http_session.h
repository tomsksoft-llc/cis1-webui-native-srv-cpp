#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "router.h"

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

class http_session : public std::enable_shared_from_this<http_session>
{
    tcp::socket socket_;
    net::strand<
        net::io_context::executor_type> strand_;
    net::steady_timer timer_;
    beast::flat_buffer buffer_;
    std::shared_ptr<router const> router_;
    http::request<http::string_body> req_;
    http_session_queue queue_;
    friend class http_session_queue;
public:
    // Take ownership of the socket
    explicit http_session(
        tcp::socket socket,
        std::shared_ptr<router const> const& router);

    // Start the asynchronous operation
    void run();

    void do_read();

    // Called when the timer expires.
    void on_timer(beast::error_code ec);

    void on_read(beast::error_code ec);

    void on_write(beast::error_code ec, bool close);

    void do_close();
};

template<bool isRequest, class Body, class Fields>
void http_session_queue::send(http::message<isRequest, Body, Fields>&& msg)
{
    // This holds a work item
    struct work_impl : work
    {
        http_session& self_;
        http::message<isRequest, Body, Fields> msg_;

        work_impl(
            http_session& self,
            http::message<isRequest, Body, Fields>&& msg)
            : self_(self)
            , msg_(std::move(msg))
        {
        }

        void
        operator()()
        {
            http::async_write(
                self_.socket_,
                msg_,
                net::bind_executor(
                    self_.strand_,
                    std::bind(
                        &http_session::on_write,
                        self_.shared_from_this(),
                        std::placeholders::_1,
                        msg_.need_eof())));
        }
    };

    // Allocate and store the work
    items_.push_back(
        boost::make_unique<work_impl>(self_, std::move(msg)));

    // If there was no previous work, start this one
    if(items_.size() == 1)
    {
        (*items_.front())();
    }
}
