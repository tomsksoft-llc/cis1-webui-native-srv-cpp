/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket::dto
{

struct user_permissions_get
{
    std::string username;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_permissions_get>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("permissions"),
                        CT_STRING("get"))
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&user_permissions_get::username>{})
                .done();
    }
};

} // namespace websocket::dto
