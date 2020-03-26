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

struct admin_project_permission_get_success
{
    struct user
    {
        std::string email;
        bool admin;
        bool read;
        bool write;
        bool execute;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<user>()
                    .add_field(
                            CT_STRING("email"),
                            ptr_v<&user::email>{})
                    .add_field(
                            CT_STRING("admin"),
                            ptr_v<&user::admin>{})
                    .add_field(
                            CT_STRING("read"),
                            ptr_v<&user::read>{})
                    .add_field(
                            CT_STRING("write"),
                            ptr_v<&user::write>{})
                    .add_field(
                            CT_STRING("execute"),
                            ptr_v<&user::execute>{})
                    .done();
        }
    };

    std::vector<user> users;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<admin_project_permission_get_success>()
                .set_name(
                        CT_STRING("admin"),
                        CT_STRING("project"),
                        CT_STRING("permission"),
                        CT_STRING("get"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("users"),
                        ptr_v<&admin_project_permission_get_success::users>{})
                .done();
    }
};

} // namespace websocket::dto
