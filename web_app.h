#pragma once

#include <functional>
#include <map>
#include <string>
#include <any>
#include <memory>

#include <boost/asio.hpp>

#include "net/http_session.h"

class web_app
    : public std::enable_shared_from_this<web_app>
{
public:
    enum class handle_result
    {
        done,
        next,
        error
    };
    using request_t = http::request<http::string_body>;
    using queue_t = http_session::queue;
    using context_t = std::map<std::string, std::any>;
    using handler_t = 
        std::function<handle_result(
            request_t&,
            queue_t&,
            context_t&)>;
    using ws_handler_t = 
        std::function<handle_result(
                request_t&,
                tcp::socket&,
                context_t&)>;
private:
    boost::asio::io_context& ioc_;
    std::vector<handler_t> handlers_;
    std::vector<ws_handler_t> ws_handlers_;
    handler_t error_handler_;
    ws_handler_t ws_error_handler_;
public:
    web_app(boost::asio::io_context& ioc);
    void append_handler(const handler_t& handler);
    void append_ws_handler(const ws_handler_t& handler);
    void set_error_handler(const handler_t& handler);
    void set_ws_error_handler(const ws_handler_t& handler);
    void listen(const tcp::endpoint& endpoint);
    void handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue) const;
    void handle_upgrade(
            tcp::socket&& socket,
            http::request<http::string_body>&& req) const;
};
