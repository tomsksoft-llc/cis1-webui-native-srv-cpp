/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket::dto
{

struct group_error_doesnt_exist
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<group_error_doesnt_exist>()
                .set_name(
                        CT_STRING("group"),
                        CT_STRING("error"),
                        CT_STRING("doesnt_exist"))
                .done();
    }
};

} // namespace websocket::dto
