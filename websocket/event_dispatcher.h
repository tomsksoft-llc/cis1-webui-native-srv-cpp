#pragma once

#include <functional>
#include <map>

#include <rapidjson/document.h>

#include "net/queued_websocket_session.h"
#include "request_context.h"
#include "event_list.h"
#include "base_event_handler.h"
#include "transaction.h"

#include "tpl_reflect/json_engine.h"

namespace websocket
{

class event_dispatcher
{
public:
    template <class ReqType>
    using default_event_handler_t = void(
            request_context&,
            const ReqType&,
            transaction);

    void dispatch(
            request_context& ctx,
            bool text,
            boost::beast::flat_buffer& buffer,
            size_t bytes_transferred,
            const std::shared_ptr<net::websocket_queue>& queue);

    template <class ReqType>
    void add_event_handler(
            request_id event_id,
            const std::function<default_event_handler_t<ReqType>>& cb)
    {
        event_handlers_.insert({
                (int)event_id,
                [cb, event_id](const std::shared_ptr<net::websocket_queue>& queue,
                     request_context& ctx,
                     const rapidjson::Value& json,
                     uint64_t transaction_id)
                {
                    ReqType req;
                    const auto& conv = ReqType::get_converter();
                    if(    conv.template has<json::engine>(json)
                        && conv.template get<json::engine>(json, req))
                    {
                        cb(ctx, req, transaction(
                                    queue,
                                    transaction_id,
                                    static_cast<int32_t>(event_id) + 1));
                    }

                    //TODO send unknown event id error
                }});
    }
private:
    using event_handler_t = void(
            const std::shared_ptr<net::websocket_queue>& queue,
            request_context& ctx,
            const rapidjson::Value& json,
            uint64_t transaction_id);

    std::map<int, std::function<event_handler_t>> event_handlers_;
};

} // namespace websocket
