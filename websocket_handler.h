#pragma once

#include <functional>
#include <string>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

class websocket_handler
{
    std::map<int, std::function<void()>> event_handlers_;
public:
    void handle(
            bool text,
            beast::flat_buffer& buffer,
            size_t bytes_transferred,
            websocket_queue& queue)
    {
        if(text)
        {
            using namespace boost::iostreams;

            std::string str;
            str.resize(bytes_transferred);
            boost::asio::buffer_copy(
                    boost::asio::buffer(str.data(), bytes_transferred),
                    buffer.data(),
                    bytes_transferred);
            
            basic_array_source<char> input_source(str.data(), bytes_transferred);
            stream<basic_array_source<char> > input_stream(input_source);
         
            pt::ptree tree;
            try
            {
                read_json(input_stream, tree);
            }
            catch(...) //TODO
            {
                //send json_parse_err
                return;
            }
            if(auto event_id = tree.get_optional<int>("eventId"); event_id)
            {
                if(auto it = event_handlers_.find(event_id.value()); it != event_handlers_.end())
                {
                    it->second();
                }
            }
            //auto reply = std::make_shared<std::string>("reply text");
            //queue.send_text(boost::asio::buffer(reply->data(), reply->size()), [reply](){});
        }
    }
    void add_event(
            int event_id,
            std::function<void()> cb)
    {
        event_handlers_.try_emplace(event_id, cb);
    }
};
