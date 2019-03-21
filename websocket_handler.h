#pragma once

#include <functional>
#include <map>

#include <rapidjson/document.h>

#include "net/queued_websocket_session.h"
#include "web_app.h" //for web_app:context_t
#include "websocket_event_list.h"

class websocket_handler
{
public:
    using event_handler_t = void(
            const rapidjson::Document&,
            websocket_queue&,
            web_app::context_t& ctx);
    void handle(
            web_app::context_t& ctx,
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
