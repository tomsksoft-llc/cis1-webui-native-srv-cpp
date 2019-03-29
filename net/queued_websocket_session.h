#pragma once

#include <deque>

#include "basic_websocket_session.h"

class queued_websocket_session;

class websocket_queue
{
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
    void send();
public:
    websocket_queue(queued_websocket_session& self);
    void send_binary(boost::asio::const_buffer buffer, std::function<void()> on_write);
    void send_text(boost::asio::const_buffer buffer, std::function<void()> on_write);
    bool is_full();
    bool on_write();
};

class queued_websocket_session
    : public basic_websocket_session
{
public:
    using request_handler_t = std::function<void(bool, beast::flat_buffer&, size_t, std::shared_ptr<websocket_queue>)>;
private:
    request_handler_t handler_;
    websocket_queue queue_;
    beast::flat_buffer in_buffer_;
    friend class websocket_queue;
public:
    static void accept_handler(
            tcp::socket&& socket,
            http::request<http::string_body>&& req,
            request_handler_t handler);
    explicit queued_websocket_session(tcp::socket socket, request_handler_t handler);
#ifndef NDEBUG
    ~queued_websocket_session();
#endif
    void on_accept_success() override;
    std::shared_ptr<queued_websocket_session> shared_from_this();

    void do_read();

    void on_read(
        beast::error_code ec,
        std::size_t bytes_transferred);

    void on_write(
        beast::error_code ec,
        std::size_t bytes_transferred);

    std::shared_ptr<websocket_queue> get_queue();
};
