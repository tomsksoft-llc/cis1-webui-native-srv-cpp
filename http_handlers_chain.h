#pragma once

#include <functional>
#include <memory>

#include <boost/asio.hpp>

#include "net/http_handler_interface.h"
#include "request_context.h"
#include "handle_result.h"

using tcp = boost::asio::ip::tcp;

class http_handlers_chain
    : public std::enable_shared_from_this<http_handlers_chain>
    , public net::http_handler_interface
{
public:
    using request_header_t = boost::beast::http::request<boost::beast::http::empty_body>;
    using queue_t = net::http_session::queue;
    using context_t = request_context;
    using handler_t = 
        std::function<handle_result(
            request_header_t&,
            context_t&,
            net::http_session::request_reader&,
            queue_t&)>;
    using ws_handler_t = 
        std::function<handle_result(
            request_header_t&,
            context_t&,
            tcp::socket&)>;
    using error_handler_t = 
        std::function<void(
            request_header_t&,
            context_t&,
            queue_t&)>;
private:
    std::vector<handler_t> handlers_;
    std::vector<ws_handler_t> ws_handlers_;
    error_handler_t error_handler_;
public:
    http_handlers_chain();
    void append_handler(const handler_t& handler);
    void append_ws_handler(const ws_handler_t& handler);
    void set_error_handler(const error_handler_t& handler);
    void listen(boost::asio::io_context& ioc, const tcp::endpoint& endpoint);
    void handle_header(
            request_header_t& req,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue) const;
    void handle_upgrade(
            tcp::socket&& socket,
            request_header_t&& req,
            net::http_session::queue& queue) const;
};
