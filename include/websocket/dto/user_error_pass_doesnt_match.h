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

struct user_error_pass_doesnt_match
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_error_pass_doesnt_match>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("error"),
                        CT_STRING("pass_doesnt_match"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
