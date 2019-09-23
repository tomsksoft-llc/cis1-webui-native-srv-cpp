#pragma once

#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct fs_entry_list_success
{
    std::vector<fs_entry> entries;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_list_success>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("list"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&fs_entry_list_success::entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
