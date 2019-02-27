#pragma once

#include <functional>

#include "http_session_queue.h"

class login_handler
{
    std::function<std::string(const std::string&, const std::string&)> authenticate_;
    std::function<bool(const std::string&)> authorize_;
public:
    login_handler(
            std::function<std::string(const std::string&, const std::string&)> authenticate_fn,
            std::function<bool(const std::string&)> authorize_fn);
    void handle(
            http::request<http::string_body>&& req,
            http_session_queue& queue);
    bool authorize(http::request<http::string_body>& req);
};
