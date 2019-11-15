#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_project_add
{
    std::string project;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_project_add>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("project"),
                        CT_STRING("add"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_project_add::project>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
