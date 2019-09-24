#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_move_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_move_success>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("move"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace dto

} // namespace websocket