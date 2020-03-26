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

struct user_change_pass
{
    std::string email;
    std::string old_password;
    std::string new_password;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_change_pass>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("change_pass"))
                .add_field(
                        CT_STRING("email"),
                        ptr_v<&user_change_pass::email>{})
                .add_field(
                        CT_STRING("oldPassword"),
                        ptr_v<&user_change_pass::old_password>{})
                .add_field(
                        CT_STRING("newPassword"),
                        ptr_v<&user_change_pass::new_password>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
