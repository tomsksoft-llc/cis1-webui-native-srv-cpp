#pragma once

#include "http_session.h"

class file_handler
{
    const std::string doc_root_;
public:
    file_handler(const std::string& doc_root);
    void handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue,
            const std::string only_path = "");
};
