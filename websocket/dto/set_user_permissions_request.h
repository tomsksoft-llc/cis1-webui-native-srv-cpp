#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct set_user_permissions_request
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
                            ptr_v<&permission::project>{})
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

    std::string username;
    std::vector<permission> permissions;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<set_user_permissions_request>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&set_user_permissions_request::username>{})
                .add_field(
                        CT_STRING("permissions"),
                        ptr_v<&set_user_permissions_request::permissions>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
