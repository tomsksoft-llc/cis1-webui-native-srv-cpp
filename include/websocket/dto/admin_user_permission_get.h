/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct admin_user_permission_get
{
    std::string email;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<admin_user_permission_get>()
                .set_name(
                        CT_STRING("admin"),
                        CT_STRING("user"),
                        CT_STRING("permission"),
                        CT_STRING("get"))
                .add_field(
                        CT_STRING("email"),
                        ptr_v<&admin_user_permission_get::email>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
