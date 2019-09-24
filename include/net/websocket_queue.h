#pragma once

#include <functional>
#include <deque>

#include <boost/asio/buffer.hpp>
#include <boost/asio/executor.hpp>

#include "websocket/queue_interface.h"

namespace net
{

class queued_websocket_session;

class websocket_queue
    : public websocket::queue_interface
{
public:
    friend class queued_websocket_session;

    websocket_queue(queued_websocket_session& self);
    ~websocket_queue() = default;

    void send_binary(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write) override;
    void send_text(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write) override;
    bool is_full() override;
    boost::asio::executor get_executor() override;
private:
    enum
    {
        limit = 64
    };
    struct message
    {
        bool text;
        boost::asio::const_buffer buffer;
        std::function<void()> on_write;
    };
    queued_websocket_session& self_;
    std::deque<message> messages_;

    bool on_write();
    void send();
};

} // namespace net
