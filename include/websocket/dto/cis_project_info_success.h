#pragma once

#include <vector>
#include <string>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct cis_project_info_success
{
    std::vector<fs_entry> fs_entries;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_project_info_success>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("project"),
                        CT_STRING("info"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&cis_project_info_success::fs_entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
