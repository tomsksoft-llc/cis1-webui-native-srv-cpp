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

struct fs_entry_set_executable_flag_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_set_executable_flag_success>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("set_executable_flag"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
