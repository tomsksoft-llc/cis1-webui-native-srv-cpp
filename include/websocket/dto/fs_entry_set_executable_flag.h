/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_set_executable_flag
{
    std::string path;
    bool executable;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_set_executable_flag>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("set_executable_flag"))
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&fs_entry_set_executable_flag::path>{})
                .add_field(
                        CT_STRING("executable"),
                        ptr_v<&fs_entry_set_executable_flag::executable>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
