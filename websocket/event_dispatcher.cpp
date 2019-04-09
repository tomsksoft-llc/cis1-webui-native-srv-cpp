#include "event_dispatcher.h"

#include <string>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "event_list.h"

#ifndef NDEBUG
#include <iostream>
#endif

void send_error(
        std::shared_ptr<net::websocket_queue>& queue,
        websocket::response_id event_id,
        std::string error_string)
{
    rapidjson::Document document;
    rapidjson::Value value;
    document.SetObject();
    value.SetInt(static_cast<int>(event_id));
    document.AddMember("eventId", value, document.GetAllocator());
    rapidjson::Value data_value;
    data_value.SetObject();
    value.SetString(error_string.c_str(), error_string.length(), document.GetAllocator());
    data_value.AddMember("errorMessage", value, document.GetAllocator());
    document.AddMember("data", data_value, document.GetAllocator());
    auto buffer = std::make_shared<rapidjson::StringBuffer>();
    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
    document.Accept(writer);
    queue->send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

namespace websocket
{

void event_dispatcher::dispatch(
        request_context& ctx,
        bool text,
        boost::beast::flat_buffer& buffer,
        size_t bytes_transferred,
        std::shared_ptr<net::websocket_queue> queue)
{
    if(text)
    {     
        std::string str;
        str.resize(bytes_transferred);
        boost::asio::buffer_copy(
                boost::asio::buffer(str.data(), bytes_transferred),
                buffer.data(),
                bytes_transferred);
#ifndef NDEBUG
        const auto& user = ctx.username;
        std::cout << "[" << user << "]:" << str << std::endl;
#endif
        rapidjson::Document request;
        request.Parse(str.c_str());
        
        if(request.HasParseError())
        {
            send_error(queue, response_id::generic_error, "Invalid JSON.");
            return;
        }

        if(   request.HasMember("eventId") && request["eventId"].IsInt()
           && request.HasMember("transanctionId") && request["transanctionId"].IsInt())
        {
            auto event_id = request["eventId"].GetInt();
            if(auto it = event_handlers_.find(request["eventId"].GetInt());
                    it != event_handlers_.end())
            {
                if(request.HasMember("data") && request["data"].IsObject())
                {
                    it->second->handle(
                            queue,
                            ctx,
                            request["data"],
                            static_cast<request_id>(event_id),
                            request["transanctionId"].GetInt());
                }
                else
                {
                    send_error(
                            queue,
                            static_cast<response_id>(event_id),
                            "Request doesn't contain 'data' member.");
                    return;
                }
            }
        }
        else
        {
            send_error(
                    queue,
                    response_id::generic_error,
                    "Request doesn't contain 'eventId' or 'transanctionId' member.");
            return;
        }
    }
}

void event_dispatcher::add_event_handler(
        request_id event_id,
        std::function<default_event_handler_t> cb)
{
    class handler
        : public base_event_handler
    {
        std::function<default_event_handler_t> process_;
    public:
        handler(std::function<default_event_handler_t> p)
            : process_(p)
        {}
        std::optional<std::string> process(
                request_context& ctx,
                const rapidjson::Value& request_data,
                rapidjson::Value& response_data,
                rapidjson::Document::AllocatorType& allocator) override
        {
            return process_(ctx, request_data, response_data, allocator);
        }
    };
    event_handlers_.try_emplace(static_cast<int>(event_id), std::make_shared<handler>(cb));
}

} // namespace websocket
