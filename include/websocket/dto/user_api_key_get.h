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

struct user_api_key_get
{
    std::string email;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_api_key_get>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("api_key"),
                        CT_STRING("get"))
                .add_field(
                        CT_STRING("email"),
                        ptr_v<&user_api_key_get::email>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
