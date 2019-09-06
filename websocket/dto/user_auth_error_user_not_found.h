#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_auth_error_user_not_found
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_auth_error_user_not_found>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
