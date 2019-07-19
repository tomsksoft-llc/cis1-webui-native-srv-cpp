#include "event_dispatcher.h"

#include <string>
#include <variant>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "event_list.h"
#include "const_stream_adapter.h"

#include "tpl_reflect/meta_converter.h"
#include "tpl_reflect/json_engine.h"
#include "tpl_helpers/overloaded.h"

namespace websocket
{

enum class error
{
    parse,
    invalid_json,
};

struct protocol_message
{
    int32_t event_id;
    uint64_t transaction_id;
    rapidjson::Document data;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<protocol_message>()
                .add_field(
                        CT_STRING("eventId"),
                        ptr_v<&protocol_message::event_id>{})
                .add_field(
                        CT_STRING("transactionId"),
                        ptr_v<&protocol_message::transaction_id>{})
                .add_field(
                        CT_STRING("data"))
                .done();
    }
};

std::variant<protocol_message, error> parse_protocol_message(
        boost::beast::flat_buffer& buffer)
{
    const_stream_adapter bs(buffer.data());
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

void send_error(
        const std::shared_ptr<net::websocket_queue>& queue,
        websocket::response_id event_id,
        const std::string& error_string)
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

void event_dispatcher::dispatch(
        request_context& ctx,
        bool text,
        boost::beast::flat_buffer& buffer,
        size_t bytes_transferred,
        const std::shared_ptr<net::websocket_queue>& queue)
{
    if(text)
    {
        auto msg = parse_protocol_message(buffer);

        std::visit(
                meta::overloaded{
                [&](const error& err)
                {
                    std::cout << "err" << std::endl;
                },
                [&](const protocol_message& msg)
                {
                    if(auto it = event_handlers_.find(msg.event_id);
                            it != event_handlers_.end())
                    {
                        (it->second)(
                                queue,
                                ctx,
                                msg.data,
                                msg.transaction_id);
                    }
                    else
                    {
                        std::cout << "unknown event_id" << std::endl;
                    }
                }},
                msg);
    }
}

} // namespace websocket
