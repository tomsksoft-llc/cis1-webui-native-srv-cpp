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

struct user_permissions_get_error_doesnt_exist
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_permissions_get_error_doesnt_exist>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("permissions"),
                        CT_STRING("get"),
                        CT_STRING("error"),
                        CT_STRING("doesnt_exist"))
                .done();
    }
};

} // namespace websocket::dto
