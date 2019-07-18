#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct disable_user_request
{
    std::string username;
    bool state;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<disable_user_request>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&disable_user_request::username>{})
                .add_field(
                        CT_STRING("state"),
                        ptr_v<&disable_user_request::state>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
