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

struct admin_project_permission_get
{
    std::string project;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<admin_project_permission_get>()
                .set_name(
                        CT_STRING("admin"),
                        CT_STRING("project"),
                        CT_STRING("permission"),
                        CT_STRING("get"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&admin_project_permission_get::project>{})
                .done();
    }
};

} // namespace websocket::dto
