#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct get_api_key_response
{
    std::string api_key;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_api_key_response>()
                .add_field(
                        CT_STRING("APIAccessSecretKey"),
                        ptr_v<&get_api_key_response::api_key>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
