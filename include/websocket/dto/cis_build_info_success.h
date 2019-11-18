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

struct cis_build_info_success
{
    int32_t status;
    std::string date;
    std::vector<fs_entry> fs_entries;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_build_info_success>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("build"),
                        CT_STRING("info"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("status"),
                        ptr_v<&cis_build_info_success::status>{})
                .add_field(
                        CT_STRING("date"),
                        ptr_v<&cis_build_info_success::date>{})
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&cis_build_info_success::fs_entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
