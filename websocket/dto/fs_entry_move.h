#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_move
{
    std::string old_path;
    std::string new_path;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_move>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("move"))
                .add_field(
                        CT_STRING("oldPath"),
                        ptr_v<&fs_entry_move::old_path>{})
                .add_field(
                        CT_STRING("newPath"),
                        ptr_v<&fs_entry_move::new_path>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
