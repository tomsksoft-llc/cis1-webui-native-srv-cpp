#pragma once

#include "websocket_handler.h"
#include "auth_manager.h"

#include <rapidjson/document.h>

void handle_auth(
        std::shared_ptr<auth_manager> authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        web_app::context_t& ctx);

void handle_token(
        std::shared_ptr<auth_manager> authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        web_app::context_t& ctx);
