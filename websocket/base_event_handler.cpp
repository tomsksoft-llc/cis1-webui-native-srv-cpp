#include "base_event_handler.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace websocket
{

void base_event_handler::handle(
        const std::shared_ptr<net::websocket_queue>& queue,
        request_context& ctx,
        const rapidjson::Value& request_data,
        request_id req_id,
        uint64_t transanction_id)
{
    rapidjson::Document response;
    rapidjson::Value value;
    response.SetObject();
    value.SetInt(static_cast<int>(req_id) + 1);
    response.AddMember("eventId", value, response.GetAllocator());
    value.SetInt(transanction_id);
    response.AddMember("transanctionId", value, response.GetAllocator());
    response.AddMember(
            "data",
            value.SetObject(),
            response.GetAllocator());

    auto error = process(ctx, request_data, response["data"], response.GetAllocator());

    if(error)
    {
        auto& error_string = error.value();
        value.SetString(
                error_string.c_str(),
                error_string.length(),
                response.GetAllocator());
        response["data"].AddMember("errorMessage", value, response.GetAllocator());
    }
    else
    {
        response["data"].AddMember("errorMessage", "", response.GetAllocator());
    }

    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    response.Accept(writer);

    queue->send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

} // namespace websocket
