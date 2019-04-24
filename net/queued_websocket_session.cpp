#include "queued_websocket_session.h"

#include "fail.h"

#ifndef NDEBUG
#include <iostream>
#endif

namespace net
{

websocket_queue::websocket_queue(queued_websocket_session& self)
    : self_(self)
{
    static_assert(limit > 0, "queue limit must be positive");
}

void websocket_queue::send()
{
        self_.ws_.text(messages_.front().text);
        self_.ws_.async_write(
                messages_.front().buffer,
                boost::asio::bind_executor(
                        self_.strand_,
                        std::bind(
                                &queued_websocket_session::on_write,
                                self_.shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2)));
}

void websocket_queue::send_text(boost::asio::const_buffer buffer, std::function<void()> on_write)
{
    messages_.push_back({true, buffer, std::move(on_write)});

    if(messages_.size() == 1)
    {
        send();
    }
}

bool websocket_queue::is_full()
{
    return messages_.size() >= limit;
}

bool websocket_queue::on_write()
{
    BOOST_ASSERT(!messages_.empty());
    auto const was_full = is_full();
    messages_.front().on_write();
    messages_.erase(messages_.begin());

    if(!messages_.empty())
    {
        send();
    }

    return was_full;
}

void queued_websocket_session::accept_handler(
        boost::asio::ip::tcp::socket&& socket,
        boost::beast::http::request<boost::beast::http::empty_body>&& req,
        request_handler_t handler)
{
    std::make_shared<queued_websocket_session>(
        std::move(socket), std::move(handler))->do_accept(std::move(req));
#ifndef NDEBUG
    std::cout << "accept_handler()" << std::endl;
#endif
}

queued_websocket_session::queued_websocket_session(boost::asio::ip::tcp::socket socket, request_handler_t handler)
    : basic_websocket_session(std::move(socket))
    , handler_(std::move(handler))
    , queue_(*this)
{}

#ifndef NDEBUG
queued_websocket_session::~queued_websocket_session()
{
    std::cout << "~queued_websocket_session()" << std::endl;
}
#endif

void queued_websocket_session::on_accept_success()
{
    do_read();
}

std::shared_ptr<queued_websocket_session> queued_websocket_session::shared_from_this()
{
    return shared_from_base<queued_websocket_session>();
}

void queued_websocket_session::do_read()
{
    // Read a message into our buffer
    ws_.async_read(
        in_buffer_,
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &queued_websocket_session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2)));
}

void queued_websocket_session::on_read(
        boost::beast::error_code ec,
        std::size_t bytes_transferred)
{
    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
    {
        return;
    }
    // This indicates that the websocket_session was closed
    if(ec == boost::beast::websocket::error::closed)
    {
#ifndef NDEBUG
        std::cout << "websocket::error::closed" << std::endl;
#endif
        timer_.expires_after(std::chrono::seconds(0));
        return;
    }
    if(ec)
    {
        fail(ec, "read");
    }
    // Note that there is activity
    activity();

    handler_(ws_.got_text(), in_buffer_, bytes_transferred, get_queue());

    in_buffer_.consume(in_buffer_.size());

    if(!queue_.is_full())
    {
        do_read();
    }
}

void queued_websocket_session::on_write(
        boost::beast::error_code ec,
        std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // Happens when the timer closes the socket
    if(ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    if(ec)
    {
        return fail(ec, "write");
    }

    if(queue_.on_write())
    {
        do_read();
    }
}

std::shared_ptr<websocket_queue> queued_websocket_session::get_queue()
{
    return std::shared_ptr<websocket_queue>(shared_from_this(), &queue_);
}

} // namespace net
