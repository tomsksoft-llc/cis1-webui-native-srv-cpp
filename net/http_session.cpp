#include "http_session.h"

#include <chrono>

#include "fail.h"
#include "web_app.h"

namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>

http_session::http_session(
    tcp::socket socket,
    const std::shared_ptr<web_app const>& app)
    : socket_(std::move(socket))
    , strand_(socket_.get_executor())
    , timer_(socket_.get_executor().context(),
        (std::chrono::steady_clock::time_point::max)())
    , app_(app)
    , queue_(*this)
{
}

void http_session::run()
{
    // Make sure we run on the strand
    if(!strand_.running_in_this_thread())
    {
        return net::post(
            net::bind_executor(
                strand_,
                std::bind(
                    &http_session::run,
                    shared_from_this())));
    }

    // Run the timer. The timer is operated
    // continuously, this simplifies the code.
    on_timer({});

    do_read();
}

void http_session::do_read()
{
    // Set the timer
    timer_.expires_after(std::chrono::seconds(15));

    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    // Read a request
    http::async_read(socket_, buffer_, req_,
        net::bind_executor(
            strand_,
            std::bind(
                &http_session::on_read,
                shared_from_this(),
                std::placeholders::_1)));
}

void http_session::on_timer(beast::error_code ec)
{
    if(ec && ec != net::error::operation_aborted)
    {
        return fail(ec, "timer");
    }

    // Check if this has been upgraded to Websocket
    if(timer_.expiry() == (std::chrono::steady_clock::time_point::min)())
    {
        return;
    }

    // Verify that the timer really expired since the deadline may have moved.
    if(timer_.expiry() <= std::chrono::steady_clock::now())
    {
        // Closing the socket cancels all outstanding operations. They
        // will complete with net::error::operation_aborted
        socket_.shutdown(tcp::socket::shutdown_both, ec);
        socket_.close(ec);
        return;
    }

    // Wait on the timer
    timer_.async_wait(
        net::bind_executor(
            strand_,
            std::bind(
                &http_session::on_timer,
                shared_from_this(),
                std::placeholders::_1)));
}

void http_session::on_read(beast::error_code ec)
{
    // Happens when the timer closes the socket
    if(ec == net::error::operation_aborted)
    {
        return;
    }

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
    {
        return do_close();
    }

    if(ec)
    {
        return fail(ec, "read");
    }

    // See if it is a WebSocket Upgrade
    if(websocket::is_upgrade(req_))
    {
        // Make timer expire immediately, by setting expiry to time_point::min we can detect
        // the upgrade to websocket in the timer handler
        timer_.expires_at((std::chrono::steady_clock::time_point::min)());

        // Create a WebSocket websocket_session by transferring the socket
        app_->handle_upgrade(std::move(socket_), std::move(req_));
        return;
    }

    // Send the response
    app_->handle(std::move(req_), queue_);

    // If we aren't at the queue limit, try to pipeline another request
    if(!queue_.is_full())
    {
        do_read();
    }
}

void http_session::on_write(beast::error_code ec, bool close)
{
    // Happens when the timer closes the socket
    if(ec == net::error::operation_aborted)
    {
        return;
    }

    if(ec)
    {
        return fail(ec, "write");
    }

    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // Inform the queue that a write completed
    if(queue_.on_write())
    {
        // Read another request
        do_read();
    }
}

void http_session::do_close()
{
    // Send a TCP shutdown
    beast::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}
