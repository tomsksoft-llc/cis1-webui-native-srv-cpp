/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket::dto
{

struct user_auth_add
{
    std::string username;
    std::string pass;
    std::string email;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_auth_add>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("auth"),
                        CT_STRING("add"))
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&user_auth_add::username>{})
                .add_field(
                        CT_STRING("pass"),
                        ptr_v<&user_auth_add::pass>{})
                .add_field(
                        CT_STRING("email"),
                        ptr_v<&user_auth_add::email>{})
                .done();
    }
};

} // namespace websocket::dto
