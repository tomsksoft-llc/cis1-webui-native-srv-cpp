#pragma once

#include "tpl_reflect/meta_converter.h"
#include "tpl_reflect/json_engine.h"

namespace websocket
{

struct protocol_message
{
    std::string event;
    uint64_t transaction_id;
    rapidjson::Document data;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<protocol_message>()
                .add_field(
                        CT_STRING("event"),
                        ptr_v<&protocol_message::event>{})
                .add_field(
                        CT_STRING("transactionId"),
                        ptr_v<&protocol_message::transaction_id>{})
                .add_field(
                        CT_STRING("data"))
                .done();
    }
};

} // namespace websocket
