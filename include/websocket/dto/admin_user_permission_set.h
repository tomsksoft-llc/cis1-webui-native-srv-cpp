/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "path_utils.h"

namespace websocket
{

namespace dto
{

struct admin_user_permission_set
{
    struct permission
    {
        std::string project;
        bool read;
        bool write;
        bool execute;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<permission>()
                    .add_field(
                            CT_STRING("project"),
                            ptr_v<&permission::project>{},
                            [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                    .add_field(
                            CT_STRING("read"),
                            ptr_v<&permission::read>{})
                    .add_field(
                            CT_STRING("write"),
                            ptr_v<&permission::write>{})
                    .add_field(
                            CT_STRING("execute"),
                            ptr_v<&permission::execute>{})
                    .done();
        }
    };

    std::string email;
    std::vector<permission> permissions;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<admin_user_permission_set>()
                .set_name(
                        CT_STRING("admin"),
                        CT_STRING("user"),
                        CT_STRING("permission"),
                        CT_STRING("set"))
                .add_field(
                        CT_STRING("email"),
                        ptr_v<&admin_user_permission_set::email>{})
                .add_field(
                        CT_STRING("permissions"),
                        ptr_v<&admin_user_permission_set::permissions>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
