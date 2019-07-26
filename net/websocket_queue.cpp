#include "websocket_queue.h"

#include "queued_websocket_session.h"

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

void websocket_queue::send_text(
        boost::asio::const_buffer buffer,
        std::function<void()> on_write)
{
    messages_.push_back({true, buffer, std::move(on_write)});

    if(messages_.size() == 1)
    {
        send();
    }
}

void websocket_queue::send_binary(
        boost::asio::const_buffer buffer,
        std::function<void()> on_write)
{
    messages_.push_back({false, buffer, std::move(on_write)});

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

boost::asio::executor websocket_queue::get_executor()
{
    return self_.strand_;
}

} // namespace net
