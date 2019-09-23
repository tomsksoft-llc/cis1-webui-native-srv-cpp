#pragma once

#include <functional>

#include <boost/asio/executor.hpp>
#include <boost/asio/buffer.hpp>

namespace websocket
{

struct queue_interface
{
    virtual ~queue_interface() = default;
    virtual void send_binary(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write) = 0;
    virtual void send_text(
            boost::asio::const_buffer buffer,
            std::function<void()> on_write) = 0;
    virtual bool is_full() = 0;
    virtual boost::asio::executor get_executor() = 0;
};

};
