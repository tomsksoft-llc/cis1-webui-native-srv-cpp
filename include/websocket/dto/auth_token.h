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

struct auth_token
{
    std::string token;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_token>()
                .set_name(
                        CT_STRING("auth"),
                        CT_STRING("token"))
                .add_field(
                        CT_STRING("token"),
                        ptr_v<&auth_token::token>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
