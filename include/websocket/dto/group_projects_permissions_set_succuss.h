/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2020 TomskSoft LLC
 *   (c) Sergey Boyko [bso@tomsksoft.com]
 *
 */

#pragma once

namespace websocket::dto
{

struct group_projects_permissions_set_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<group_projects_permissions_set_success>()
                .set_name(
                        CT_STRING("group"),
                        CT_STRING("projects"),
                        CT_STRING("permissions"),
                        CT_STRING("set"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace websocket::dto
