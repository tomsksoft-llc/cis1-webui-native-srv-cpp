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

struct admin_user_add_error_incorrect_credentials
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<admin_user_add_error_incorrect_credentials>()
                .set_name(
                        CT_STRING("admin"),
                        CT_STRING("user"),
                        CT_STRING("add"),
                        CT_STRING("error"),
                        CT_STRING("incorrect_credentials"))
                .done();
    }
};

} // namespace
