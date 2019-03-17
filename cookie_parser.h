#pragma once

#include "web_app.h"

class cookie_parser
{
public:
    static web_app::handle_result parse(
            web_app::request_t& req,
            web_app::queue_t& queue,
            web_app::context_t& ctx);
};
