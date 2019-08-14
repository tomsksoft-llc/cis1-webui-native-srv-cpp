#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry_refresh
{
    std::string path;
    bool recursive;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_refresh>()
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&fs_entry_refresh::path>{})
                .add_field(
                        CT_STRING("recursive"),
                        ptr_v<&fs_entry_refresh::recursive>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
