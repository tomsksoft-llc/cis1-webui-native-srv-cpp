#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_auth_ban
{
    std::string username;
    bool state;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_auth_ban>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&user_auth_ban::username>{})
                .add_field(
                        CT_STRING("state"),
                        ptr_v<&user_auth_ban::state>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
