#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_auth_change_pass_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_auth_change_pass_success>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
