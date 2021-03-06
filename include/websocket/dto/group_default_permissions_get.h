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

struct group_default_permissions_get
{
    std::string group;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<group_default_permissions_get>()
                .set_name(
                        CT_STRING("group"),
                        CT_STRING("default"),
                        CT_STRING("permissions"),
                        CT_STRING("get"))
                .add_field(
                        CT_STRING("group"),
                        ptr_v<&group_default_permissions_get::group>{})
                .done();
    }
};

} // namespace websocket::dto
