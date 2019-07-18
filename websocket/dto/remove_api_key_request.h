#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct remove_api_key_request
{
    std::string username;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<remove_api_key_request>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&remove_api_key_request::username>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
