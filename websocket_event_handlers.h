#pragma once

#include "request_context.h"
#include "web_app.h"
#include "auth_manager.h"
#include "net/queued_websocket_session.h"

#include <rapidjson/document.h>

void handle_auth(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx);

void handle_token(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx);

void handle_logout(
        const std::shared_ptr<auth_manager>& authentication_handler,
        const rapidjson::Document& data,
        websocket_queue& queue,
        request_context& ctx);
