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

struct user_permission_error_access_denied
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_permission_error_access_denied>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("permission"),
                        CT_STRING("error"),
                        CT_STRING("access_denied"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
