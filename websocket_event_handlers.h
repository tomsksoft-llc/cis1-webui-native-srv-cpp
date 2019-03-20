#pragma once

#include "websocket_handler.h"
#include "auth_manager.h"

#include <rapidjson/document.h>

void handle_auth(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        web_app::context_t& ctx);

void handle_token(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        web_app::context_t& ctx);

void handle_logout(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        web_app::context_t& ctx);
