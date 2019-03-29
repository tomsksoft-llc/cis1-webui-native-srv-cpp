#pragma once

#include <string>
#include <optional>
#include <memory>

#include <rapidjson/document.h>

#include "net/queued_websocket_session.h"
#include "request_context.h"
#include "websocket_event_list.h"

class base_websocket_event_handler
{
public:
    virtual void handle(
            std::shared_ptr<websocket_queue> queue,
            request_context& ctx,
            const rapidjson::Value& request_data,
            ws_request_id request_id,
            uint64_t transanction_id);

    virtual std::optional<std::string> process(
            request_context& ctx,
            const rapidjson::Value& request_data,
            rapidjson::Value& response_data,
            rapidjson::Document::AllocatorType& allocator) = 0;
};
