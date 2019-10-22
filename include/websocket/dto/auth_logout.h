#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct auth_logout
{
    std::string token;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_logout>()
                .set_name(
                        CT_STRING("auth"),
                        CT_STRING("logout"))
                .add_field(
                        CT_STRING("token"),
                        ptr_v<&auth_logout::token>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
