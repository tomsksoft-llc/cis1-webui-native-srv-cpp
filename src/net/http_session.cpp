#include "net/http_session.h"

#include <chrono>

#include "net/fail.h"
#include "net/http_handler_interface.h"

namespace net
{

http_session::http_session(
    boost::asio::ip::tcp::socket socket,
    std::shared_ptr<http_handler_interface const> app)
    : socket_(std::move(socket))
    , strand_(socket_.get_executor())
    , timer_(socket_.get_executor().context(),
        (std::chrono::steady_clock::time_point::max)())
    , app_(std::move(app))
    , queue_(*this)
    , request_reader_(*this)
{
}

void http_session::run()
{
    // Make sure we run on the strand
    if(!strand_.running_in_this_thread())
    {
        return boost::asio::post(
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &http_session::run,
                    shared_from_this())));
    }

    // Run the timer. The timer is operated
    // continuously, this simplifies the code.
    on_timer({});

    do_read_header();
}

void http_session::do_read_header()
{
    // Set the timer
    timer_.expires_after(std::chrono::seconds(15));

    // Make the new request_parser empty before reading,
    // otherwise the operation behavior is undefined.
    request_reader_.prepare();

    // Read a request
    boost::beast::http::async_read_header(socket_, buffer_,
        request_reader_.get_header_parser(),
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &http_session::on_read_header,
                shared_from_this(),
                std::placeholders::_1)));
}

void http_session::on_timer(boost::beast::error_code ec)
{
    if(ec && ec != boost::asio::error::operation_aborted)
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
        // will complete with boost::asio::error::operation_aborted
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
        return;
    }

    // Wait on the timer
    timer_.async_wait(
            boost::asio::bind_executor(
                    strand_,
                    std::bind(
                            &http_session::on_timer,
                            shared_from_this(),
                            std::placeholders::_1)));
}

void http_session::on_read_header(boost::beast::error_code ec)
{
    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    // This means they closed the connection
    if(ec == boost::beast::http::error::end_of_stream)
    {
        return do_close();
    }

    if(ec)
    {
        return fail(ec, "read");
    }

    // See if it is a WebSocket Upgrade
    if(boost::beast::websocket::is_upgrade(request_reader_.get_header_parser().get()))
    {
        // Make timer expire immediately, by setting expiry to time_point::min we can detect
        // the upgrade to websocket in the timer handler
        timer_.expires_at((std::chrono::steady_clock::time_point::min)());

        // Create a WebSocket websocket_session by transferring the socket
        app_->handle_upgrade(
                std::move(socket_),
                std::move(request_reader_.get_header_parser().release()),
                queue_);
        return;
    }

    app_->handle_header(
            request_reader_.get_header_parser().get(),
            request_reader_,
            queue_);
}

void http_session::on_write(boost::beast::error_code ec, bool close)
{
    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
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
        do_read_header();
    }
}

void http_session::do_close()
{
    // Send a TCP shutdown
    boost::beast::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

http_session::http_session_queue::http_session_queue(http_session& self)
    : self_(self)
{
    static_assert(limit > 0, "queue limit must be positive");
}

bool http_session::http_session_queue::is_full() const
{
    return items_.size() >= limit;
}

bool http_session::http_session_queue::on_write()
{
    BOOST_ASSERT(!items_.empty());
    auto const was_full = is_full();
    items_.erase(items_.begin());
    if(! items_.empty())
    {
        (*items_.front())();
    }
    return was_full;
}

template <>
void http_session::http_request_reader::upgrade_parser<boost::beast::http::empty_body>()
{}

boost::beast::http::request_parser<boost::beast::http::empty_body>&
http_session::http_request_reader::get_header_parser()
{
    return get_parser<boost::beast::http::empty_body>();
}

void http_session::http_request_reader::prepare()
{
    done_ = false;
    parser_ = std::make_unique<parser_wrapper_impl<boost::beast::http::empty_body>>();
}

void http_session::http_request_reader::done()
{
    if(done_)
    {
        return;
    }

    upgrade_parser<beast_ext::sink_body>();
    boost::beast::http::async_read(
            self_.socket_,
            self_.buffer_,
            get_parser<beast_ext::sink_body>(),
            boost::asio::bind_executor(
                    self_.strand_,
                    std::bind(
                            &http_session::on_read_body<beast_ext::sink_body>,
                            self_.shared_from_this(),
                            nullptr,
                            std::placeholders::_1,
                            std::placeholders::_2)));
    done_ = true;
}

} // namespace net
