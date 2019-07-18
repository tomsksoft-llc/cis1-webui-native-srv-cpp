#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct remove_api_key_response
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<remove_api_key_response>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
