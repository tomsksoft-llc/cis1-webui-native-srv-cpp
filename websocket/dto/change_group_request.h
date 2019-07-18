#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct change_group_request
{
    std::string username;
    std::string group;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<change_group_request>()
                .add_field(
                        CT_STRING("username"),
                        ptr_v<&change_group_request::username>{})
                .add_field(
                        CT_STRING("group"),
                        ptr_v<&change_group_request::group>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
