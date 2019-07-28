#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct set_user_permissions_response
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<set_user_permissions_response>()
                .done();
    }
};

} // namespace dto

} // namespace websocket