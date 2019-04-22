#include "basic_websocket_session.h"

#include "fail.h"

namespace net
{

basic_websocket_session::basic_websocket_session(boost::asio::ip::tcp::socket socket)
    : ws_(std::move(socket))
    , strand_(ws_.get_executor())
    , timer_(ws_.get_executor().context(),
        (std::chrono::steady_clock::time_point::max)())
{
}

void basic_websocket_session::on_accept(boost::beast::error_code ec)
{
    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    if(ec)
    {
        return fail(ec, "accept");
    }

    this->on_accept_success();
}

void basic_websocket_session::on_timer(boost::beast::error_code ec)
{
    if(ec && ec != boost::asio::error::operation_aborted)
    {
        return fail(ec, "timer");
    }

    // See if the timer really expired since the deadline may have moved.
    if(timer_.expiry() <= std::chrono::steady_clock::now())
    {
        // If this is the first time the timer expired,
        // send a ping to see if the other end is there.
        if(ws_.is_open() && ping_state_ == 0)
        {
            // Note that we are sending a ping
            ping_state_ = 1;

            // Set the timer
            timer_.expires_after(std::chrono::seconds(15));

            // Now send the ping
            ws_.async_ping({},
                boost::asio::bind_executor(
                    strand_,
                    std::bind(
                        &basic_websocket_session::on_ping,
                        shared_from_this(),
                        std::placeholders::_1)));
        }
        else
        {
            // The timer expired while trying to handshake,
            // or we sent a ping and it never completed or
            // we never got back a control frame, so close.

            // Closing the socket cancels all outstanding operations. They
            // will complete with net::error::operation_aborted
            ws_.next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            ws_.next_layer().close(ec);
            return;
        }
    }

    // Wait on the timer
    timer_.async_wait(
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &basic_websocket_session::on_timer,
                shared_from_this(),
                std::placeholders::_1)));
}

void basic_websocket_session::activity()
{
    // Note that the connection is alive
    ping_state_ = 0;

    // Set the timer
    timer_.expires_after(std::chrono::seconds(15));
}

void basic_websocket_session::on_ping(boost::beast::error_code ec)
{
    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
    {
        return;
    }
    if(ec)
    {
        return fail(ec, "ping");
    }
    // Note that the ping was sent.
    if(ping_state_ == 1)
    {
        ping_state_ = 2;
    }
    else
    {
        // ping_state_ could have been set to 0
        // if an incoming control frame was received
        // at exactly the same time we sent a ping.
        BOOST_ASSERT(ping_state_ == 0);
    }
}

void basic_websocket_session::on_control_callback(
    boost::beast::websocket::frame_type kind,
    boost::beast::string_view payload)
{
    boost::ignore_unused(kind, payload);

    // Note that there is activity
    activity();
}

} // namespace net
