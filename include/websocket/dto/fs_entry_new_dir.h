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

struct fs_entry_new_dir
{
    std::string path;
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_new_dir>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("new_dir"))
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&fs_entry_new_dir::path>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
