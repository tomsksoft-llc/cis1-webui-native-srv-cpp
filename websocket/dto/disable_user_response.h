#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct disable_user_response
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<disable_user_response>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
