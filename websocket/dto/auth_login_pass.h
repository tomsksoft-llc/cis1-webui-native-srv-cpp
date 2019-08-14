#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct auth_login_pass
{
    std::string username;
    std::string pass;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_login_pass>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&auth_login_pass::username>{})
                .add_field(
                        CT_STRING("pass"),
                        ptr_v<&auth_login_pass::pass>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
