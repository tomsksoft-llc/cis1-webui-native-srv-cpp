#pragma once

#include "http_session_queue.h"

class file_handler
{
    const std::string doc_root_;
public:
    file_handler(const std::string& doc_root);
    void handle(
            http::request<http::string_body>&& req,
            http_session_queue& queue,
            const std::string only_path = "");
};
