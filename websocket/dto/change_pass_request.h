#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct change_pass_request
{
    std::string old_password;
    std::string new_password;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<change_pass_request>()
                .add_field(
                        CT_STRING("oldPassword"),
                        ptr_v<&change_pass_request::old_password>{})
                .add_field(
                        CT_STRING("newPassword"),
                        ptr_v<&change_pass_request::new_password>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
