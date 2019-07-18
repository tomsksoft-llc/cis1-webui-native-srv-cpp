#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct get_user_list_response
{
    struct user
    {
        std::string username;
        std::string email;
        std::string group;
        bool disabled;
        std::string api_key;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<user>()
                    .add_field(
                            CT_STRING("username"),
                            ptr_v<&user::username>{})
                    .add_field(
                            CT_STRING("email"),
                            ptr_v<&user::email>{})
                    .add_field(
                            CT_STRING("group"),
                            ptr_v<&user::group>{})
                    .add_field(
                            CT_STRING("disabled"),
                            ptr_v<&user::disabled>{})
                    .add_field(
                            CT_STRING("APIAccessSecretKey"),
                            ptr_v<&user::api_key>{})
                    .done();
        }
    };

    std::vector<user> users;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_user_list_response>()
                .add_field(
                        CT_STRING("users"),
                        ptr_v<&get_user_list_response::users>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
