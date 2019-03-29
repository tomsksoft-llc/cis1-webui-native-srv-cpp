#pragma once

#include <functional>
#include <map>

#include <rapidjson/document.h>

#include "net/queued_websocket_session.h"
#include "request_context.h"
#include "websocket_event_list.h"

class websocket_handler
{
public:
    using event_handler_t = void(
            const rapidjson::Document&,
            rapidjson::Document&,
            websocket_queue&,
            request_context& ctx);
    void handle(
            request_context& ctx,
            bool text,
            beast::flat_buffer& buffer,
            size_t bytes_transferred,
            websocket_queue& queue);
    void add_event_handler(
            ws_request_id event_id,
            std::function<event_handler_t> cb);
    void add_event(
            int event_id,
            std::function<event_handler_t> cb);
private:
    std::map<int, std::function<event_handler_t>> event_handlers_;
};
