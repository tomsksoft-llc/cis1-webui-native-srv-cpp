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

struct auth_logout_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_logout_success>()
                .set_name(
                        CT_STRING("auth"),
                        CT_STRING("logout"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
