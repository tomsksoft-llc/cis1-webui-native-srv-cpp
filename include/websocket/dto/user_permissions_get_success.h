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

struct user_permissions_get_success
{
    struct permission
    {
        std::string id;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<permission>()
                    .add_field(
                            CT_STRING("id"),
                            ptr_v<&permission::id>{})
                    .done();
        }
    };

    std::vector<permission> permissions;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_permissions_get_success>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("permissions"),
                        CT_STRING("get"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("permissions"),
                        ptr_v<&user_permissions_get_success::permissions>{})
                .done();
    }
};

} // namespace websocket::dto
