#pragma once

#include <functional>
#include <map>

#include <rapidjson/document.h>

#include "net/queued_websocket_session.h"
#include "request_context.h"
#include "websocket_event_list.h"
#include "base_websocket_event_handler.h"

class websocket_handler
{
public:
    using default_event_handler_t = std::optional<std::string>(
            request_context& ctx,
            const rapidjson::Value& request_data,
            rapidjson::Value& response_data,
            rapidjson::Document::AllocatorType& allocator);
    void handle(
            request_context& ctx,
            bool text,
            beast::flat_buffer& buffer,
            size_t bytes_transferred,
            std::shared_ptr<websocket_queue> queue);
    void add_event_handler(
            ws_request_id event_id,
            std::function<default_event_handler_t> cb);
private:
    std::map<int, std::shared_ptr<base_websocket_event_handler>> event_handlers_;
};
