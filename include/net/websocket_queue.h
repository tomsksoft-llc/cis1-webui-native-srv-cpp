#pragma once

#include <functional>
#include <deque>

#include <boost/asio/buffer.hpp>
#include <boost/asio/executor.hpp>

#include <cis1_proto_utils/queue_interface.h>

namespace net
{

class queued_websocket_session;

class websocket_queue
    : public cis1::proto_utils::queue_interface
{
public:
    friend class queued_websocket_session;

    websocket_queue(queued_websocket_session& self);

    ~websocket_queue();

    void send_binary(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write);

    void send_text(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write) override;

    bool is_full() override;

    boost::asio::executor get_executor() override;

    uint32_t add_close_handler(std::function<void()> on_close) override;

    void remove_close_handler(uint32_t id) override;

    void close() override;

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
    std::map<uint32_t, std::function<void()>> close_handlers_;
    std::deque<message> messages_;

    bool on_write();
    void send();
};

} // namespace net
