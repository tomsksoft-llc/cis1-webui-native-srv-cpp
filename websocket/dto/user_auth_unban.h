#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_auth_unban
{
    std::string username;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_auth_unban>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&user_auth_unban::username>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
