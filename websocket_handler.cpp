#include "websocket_handler.h"

#include <string>

#ifndef NDEBUG
#include <iostream>
#endif

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
        rapidjson::Document document;
        document.Parse(str.c_str());
        
        if(document.HasParseError())
        {
            //TODO send wrong json err
            return;
        }

        if(document.HasMember("eventId") && document["eventId"].IsInt())
        {
            if(auto it = event_handlers_.find(document["eventId"].GetInt());
                    it != event_handlers_.end())
            {
                if(document.HasMember("data") && document["data"].IsObject())
                {
                    rapidjson::Document data;
                    data.CopyFrom(document["data"], data.GetAllocator());
                    it->second(data, queue, ctx);
                }
                else
                {
                    //TODO send no data field error
                }
            }
        }

        //TODO send wrong event id err
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
