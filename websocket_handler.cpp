#include "websocket_handler.h"

#include <string>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "websocket_event_list.h"

#ifndef NDEBUG
#include <iostream>
#endif

void send_error(
        websocket_queue& queue,
        ws_response_id event_id,
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
    queue.send_text(
            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
            [buffer](){});
}

void websocket_handler::handle(
        request_context& ctx,
        bool text,
        beast::flat_buffer& buffer,
        size_t bytes_transferred,
        websocket_queue& queue)
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
            send_error(queue, ws_response_id::generic_error, "Invalid JSON.");
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
                    rapidjson::Document data;
                    data.CopyFrom(request["data"], data.GetAllocator());
                    rapidjson::Document response;
                    rapidjson::Value value;
                    response.SetObject();
                    value.SetInt(event_id + 1);
                    response.AddMember("eventId", value, response.GetAllocator());
                    value.SetInt(request["transanctionId"].GetInt());
                    response.AddMember("transanctionId", value, response.GetAllocator());
                    it->second(data, response, queue, ctx);
                    auto buffer = std::make_shared<rapidjson::StringBuffer>();
                    rapidjson::Writer<rapidjson::StringBuffer> writer(*buffer);
                    response.Accept(writer);
                    queue.send_text(
                            boost::asio::const_buffer(buffer->GetString(), buffer->GetSize()),
                            [buffer](){});
                }
                else
                {
                    send_error(
                            queue,
                            static_cast<ws_response_id>(event_id),
                            "Request doesn't contain 'data' member.");
                    return;
                }
            }
        }
        else
        {
            send_error(
                    queue,
                    ws_response_id::generic_error,
                    "Request doesn't contain 'eventId' or 'transanctionId' member.");
            return;
        }
    }
}

void websocket_handler::add_event_handler(
        ws_request_id event_id,
        std::function<event_handler_t> cb)
{
    add_event(static_cast<int>(event_id), cb);
}

void websocket_handler::add_event(
        int event_id,
        std::function<event_handler_t> cb)
{
    event_handlers_.try_emplace(event_id, cb);
}
