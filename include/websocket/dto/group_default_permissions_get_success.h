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

struct group_default_permissions_get_success
{
    std::string group;
    bool read;
    bool write;
    bool execute;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<group_default_permissions_get_success>()
                .set_name(
                        CT_STRING("group"),
                        CT_STRING("default"),
                        CT_STRING("permissions"),
                        CT_STRING("get"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("group"),
                        ptr_v<&group_default_permissions_get_success::group>{})
                .add_field(
                        CT_STRING("read"),
                        ptr_v<&group_default_permissions_get_success::read>{})
                .add_field(
                        CT_STRING("write"),
                        ptr_v<&group_default_permissions_get_success::write>{})
                .add_field(
                        CT_STRING("execute"),
                        ptr_v<&group_default_permissions_get_success::execute>{})
                .done();
    }
};

} // namespace websocket::dto
