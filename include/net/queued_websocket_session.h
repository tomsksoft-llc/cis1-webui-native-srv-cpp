#pragma once

#include <deque>

#include <cis1_proto_utils/queue_interface.h>

#include "basic_websocket_session.h"
#include "websocket_queue.h"

namespace net
{

class queued_websocket_session
    : public basic_websocket_session
{
public:
    using request_handler_t = std::function<void(
            bool,
            boost::asio::const_buffer, 
            size_t, 
            std::shared_ptr<cis1::proto_utils::queue_interface>)>;

    static void accept_handler(
            boost::asio::ip::tcp::socket&& socket,
            boost::beast::http::request<boost::beast::http::empty_body>&& req,
            request_handler_t handler);

    explicit queued_websocket_session(
            boost::asio::ip::tcp::socket socket,
            request_handler_t handler);

#ifndef NDEBUG
    ~queued_websocket_session();
#endif

    void on_accept_success() override;

    std::shared_ptr<queued_websocket_session> shared_from_this();

    void do_read();

    void on_read(
        boost::beast::error_code ec,
        std::size_t bytes_transferred);

    void on_write(
        boost::beast::error_code ec,
        std::size_t bytes_transferred);

    std::shared_ptr<websocket_queue> get_queue();

private:
    request_handler_t handler_;
    websocket_queue queue_;
    boost::beast::flat_buffer in_buffer_;
    friend class websocket_queue;
};

} // namespace net
