#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct auth_login_pass_response
{
    std::string token;
    std::string group;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_login_pass_response>()
                .add_field(
                        CT_STRING("token"),
                        ptr_v<&auth_login_pass_response::token>{})
                .add_field(
                        CT_STRING("group"),
                        ptr_v<&auth_login_pass_response::group>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
