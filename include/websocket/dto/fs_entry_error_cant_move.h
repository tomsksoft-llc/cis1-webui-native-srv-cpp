/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_error_cant_move
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_error_cant_move>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("error"),
                        CT_STRING("cant_move"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
