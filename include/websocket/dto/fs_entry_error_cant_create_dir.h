#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_error_cant_create_dir
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_error_cant_create_dir>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("error"),
                        CT_STRING("cant_create_dir"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
