#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_list
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_list>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
