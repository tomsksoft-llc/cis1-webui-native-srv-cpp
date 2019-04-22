#pragma once

#include "http_session.h"

namespace net
{

class http_handler_interface
{
public:
    virtual ~http_handler_interface() = default;

    virtual void handle_upgrade(
            boost::asio::ip::tcp::socket&& socket,
            boost::beast::http::request<boost::beast::http::empty_body>&& req,
            http_session::queue& queue) const = 0;
    virtual void handle_header(
            boost::beast::http::request<boost::beast::http::empty_body>& req,
            http_session::request_reader& reader,
            http_session::queue& queue) const = 0;
};

} // namespace net
