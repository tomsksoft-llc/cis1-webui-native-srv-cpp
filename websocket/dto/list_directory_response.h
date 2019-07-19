#pragma once

#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct list_directory_response
{
    std::vector<fs_entry> entries;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<list_directory_response>()
                .add_field(
                        CT_STRING("entries"),
                        ptr_v<&list_directory_response::entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
