/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket::dto
{

struct admin_user_set_admin_status_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<admin_user_set_admin_status_success>()
                .set_name(
                        CT_STRING("admin"),
                        CT_STRING("user"),
                        CT_STRING("set_admin_status"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace websocket::dto
