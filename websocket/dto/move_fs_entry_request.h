#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct move_fs_entry_request
{
    std::string old_path;
    std::string new_path;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<move_fs_entry_request>()
                .add_field(
                        CT_STRING("oldPath"),
                        ptr_v<&move_fs_entry_request::old_path>{})
                .add_field(
                        CT_STRING("newPath"),
                        ptr_v<&move_fs_entry_request::new_path>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
