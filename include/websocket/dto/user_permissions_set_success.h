#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_permissions_set_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_permissions_set_success>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("permissions"),
                        CT_STRING("set"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
