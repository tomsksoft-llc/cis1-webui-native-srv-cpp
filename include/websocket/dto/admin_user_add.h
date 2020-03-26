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

struct admin_user_add
{
    std::string email;
    std::string pass;
    bool random_pass;
    bool admin;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<admin_user_add>()
                .set_name(
                        CT_STRING("admin"),
                        CT_STRING("user"),
                        CT_STRING("add"))
                .add_field(
                        CT_STRING("email"),
                        ptr_v<&admin_user_add::email>{})
                .add_field(
                        CT_STRING("pass"),
                        ptr_v<&admin_user_add::pass>{})
                .add_field(
                        CT_STRING("random_pass"),
                        ptr_v<&admin_user_add::random_pass>{})
                .add_field(
                        CT_STRING("admin"),
                        ptr_v<&admin_user_add::admin>{})
                .done();
    }
};

} // namespace websocket::dto
