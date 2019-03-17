#pragma once

#include <functional>
#include <map>

#include <boost/property_tree/ptree.hpp>

#include "net/queued_websocket_session.h"

class websocket_handler
{
    std::map<int, std::function<void(const boost::property_tree::ptree&, websocket_queue&)>> event_handlers_;
public:
    void handle(
            bool text,
            beast::flat_buffer& buffer,
            size_t bytes_transferred,
            websocket_queue& queue);
    void add_event(
            int event_id,
            std::function<void(const boost::property_tree::ptree&, websocket_queue&)> cb);
};
