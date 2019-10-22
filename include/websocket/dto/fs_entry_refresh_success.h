#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_refresh_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_refresh_success>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("refresh"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
