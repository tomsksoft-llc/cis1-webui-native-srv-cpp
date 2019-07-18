#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct get_project_info_request
{
    std::string project;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_project_info_request>()
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&get_project_info_request::project>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
