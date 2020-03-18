/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct auth_login_success
{
    std::string token;
    bool admin;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_login_success>()
                .set_name(
                        CT_STRING("auth"),
                        CT_STRING("login"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("token"),
                        ptr_v<&auth_login_success::token>{})
                .add_field(
                        CT_STRING("admin"),
                        ptr_v<&auth_login_success::admin>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
