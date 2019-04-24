#pragma once

#include <functional>
#include <map>

#include <rapidjson/document.h>

#include "net/queued_websocket_session.h"
#include "request_context.h"
#include "event_list.h"
#include "base_event_handler.h"

namespace websocket
{

class event_dispatcher
{
public:
    using default_event_handler_t = std::optional<std::string>(
            request_context& ctx,
            const rapidjson::Value& request_data,
            rapidjson::Value& response_data,
            rapidjson::Document::AllocatorType& allocator);
    void dispatch(
            request_context& ctx,
            bool text,
            boost::beast::flat_buffer& buffer,
            size_t bytes_transferred,
            const std::shared_ptr<net::websocket_queue>& queue);
    void add_event_handler(
            request_id event_id,
            const std::function<default_event_handler_t>& cb);
private:
    std::map<int, std::shared_ptr<base_event_handler>> event_handlers_;
};

} // namespace websocket
