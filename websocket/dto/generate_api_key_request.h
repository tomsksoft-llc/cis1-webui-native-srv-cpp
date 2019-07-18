#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct generate_api_key_request
{
    std::string username;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<generate_api_key_request>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&generate_api_key_request::username>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
