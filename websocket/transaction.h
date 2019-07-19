#pragma once

#include <memory>

#include "net/queued_websocket_session.h"

#include "tpl_reflect/meta_converter.h"
#include "tpl_reflect/json_engine.h"

#include "dto/dto_to_event_id.h"

namespace websocket
{

class transaction
{
public:
    transaction(
            const std::shared_ptr<net::websocket_queue>& queue,
            uint64_t transaction_id,
            int32_t default_error_id = -1);

    template <class Payload>
    void send(const Payload& p)
    {
        if(auto queue = queue_.lock(); queue)
        {
            rapidjson::Document d;

            auto id = json::dto_to_event_id<Payload>();

            prepare_response(d, id);
            d.AddMember(
                    rapidjson::Value().SetString("errorMessage"),
                    rapidjson::Value().SetString(""),
                d.GetAllocator());
            const auto& conv = Payload::get_converter();
            conv.template set<json::engine>(
                    d["data"],
                    p,
                    d.GetAllocator());

            send(queue, d);
        }
    }

    void send_error(const std::string& err);
private:
    const uint64_t transaction_id_;
    const std::weak_ptr<net::websocket_queue> queue_;
    const int32_t default_error_id_;

    void prepare_response(rapidjson::Document& doc, int32_t id);

    void send(
            const std::shared_ptr<net::websocket_queue> queue,
            const rapidjson::Value& v);
};

} // namespace websocket
