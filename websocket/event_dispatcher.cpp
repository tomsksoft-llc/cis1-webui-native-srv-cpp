#include "event_dispatcher.h"

#include <string>
#include <variant>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "const_stream_adapter.h"
#include "protocol_message.h"
#include "tpl_helpers/overloaded.h"

#ifndef NDEBUG
#include <iostream>
#endif

namespace websocket
{

enum class error
{
    parse,
    invalid_json,
};

std::variant<protocol_message, error> parse_protocol_message(
        const boost::beast::flat_buffer& buffer,
        size_t bytes_transferred)
{
    const_stream_adapter bs(buffer.data(), bytes_transferred);
    rapidjson::Document request;
    request.ParseStream(bs);

    if(request.HasParseError())
    {
        return error::parse;
    }

    auto c = protocol_message::get_converter();
    protocol_message result;

    if(    c.has<json::engine>(request)
        && c.get<json::engine>(request, result))
    {
        result.data.CopyFrom(request["data"], result.data.GetAllocator());
        return result;
    }

    return error::invalid_json;
}

void event_dispatcher::dispatch(
        request_context& ctx,
        bool text,
        boost::beast::flat_buffer& buffer,
        size_t bytes_transferred,
        const std::shared_ptr<queue_interface>& queue)
{
    if(text)
    {
#ifndef NDEBUG
        std::cout << "[" << ctx.username << "]: "
                  << boost::beast::buffers_to_string(buffer.data())
                  << std::endl;
#endif
        auto msg = parse_protocol_message(buffer, bytes_transferred);

        std::visit(
                meta::overloaded{
                [&](const error&)
                {
                    transaction(queue, 0).send_error("Invalid json.");
                },
                [&](const protocol_message& msg)
                {
                    transaction tr(
                            queue,
                            msg.transaction_id);

                    if(auto it = event_handlers_.find(msg.event);
                            it != event_handlers_.end())
                    {
                        (it->second)(
                                ctx,
                                msg.data,
                                std::move(tr));
                    }
                    else
                    {
                        tr.send_error("Unknown eventId.");
                    }
                }},
                msg);
    }
}

} // namespace websocket
