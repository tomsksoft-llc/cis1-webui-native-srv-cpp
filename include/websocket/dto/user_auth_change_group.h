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

struct user_auth_change_group
{
    std::string username;
    std::string group;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_auth_change_group>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("auth"),
                        CT_STRING("change_group"))
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&user_auth_change_group::username>{})
                .add_field(
                        CT_STRING("group"),
                        ptr_v<&user_auth_change_group::group>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
