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

struct user_error_user_not_found
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_error_user_not_found>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("error"),
                        CT_STRING("user_not_found"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
