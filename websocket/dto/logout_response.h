#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct logout_response
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<logout_response>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
