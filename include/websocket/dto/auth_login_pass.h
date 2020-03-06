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

struct auth_login_pass
{
    std::string email;
    std::string pass;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_login_pass>()
                .set_name(
                        CT_STRING("auth"),
                        CT_STRING("login_pass"))
                .add_field(
                        CT_STRING("email"),
                        ptr_v<&auth_login_pass::email>{})
                .add_field(
                        CT_STRING("pass"),
                        ptr_v<&auth_login_pass::pass>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
