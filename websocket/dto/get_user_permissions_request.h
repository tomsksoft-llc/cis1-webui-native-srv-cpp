#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct get_user_permissions_request
{
    std::string username;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_user_permissions_request>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&get_user_permissions_request::username>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
