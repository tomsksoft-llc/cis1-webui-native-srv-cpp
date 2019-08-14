#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_remove
{
    std::string path;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_remove>()
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&fs_entry_remove::path>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
