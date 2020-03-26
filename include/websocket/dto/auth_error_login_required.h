/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket::dto
{

struct auth_error_login_required
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_error_login_required>()
                .set_name(
                        CT_STRING("auth"),
                        CT_STRING("error"),
                        CT_STRING("login_required"))
                .done();
    }
};

} // namespace websocket::dto
