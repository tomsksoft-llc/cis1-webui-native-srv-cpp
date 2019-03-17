#include "websocket_handler.h"

#include <string>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/property_tree/json_parser.hpp>

void websocket_handler::handle(
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
        stream<basic_array_source<char>> input_stream(input_source);
     
        boost::property_tree::ptree tree;
        try
        {
            boost::property_tree::read_json(input_stream, tree);
        }
        catch(...) //TODO
        {
            //send json_parse_err
            return;
        }
        //TODO validate
        if(auto event_id = tree.get_optional<int>("eventId"); event_id)
        {
            if(auto it = event_handlers_.find(event_id.value()); it != event_handlers_.end())
            {
                it->second(tree.get_child("data"), queue);
            }
        }
    }
}
void websocket_handler::add_event(
        int event_id,
        std::function<void(const boost::property_tree::ptree&, websocket_queue&)> cb)
{
    event_handlers_.try_emplace(event_id, cb);
}
