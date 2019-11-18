/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct cis_project_list_get_success
{
    std::vector<fs_entry> fs_entries;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_project_list_get_success>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("project_list"),
                        CT_STRING("get"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&cis_project_list_get_success::fs_entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
