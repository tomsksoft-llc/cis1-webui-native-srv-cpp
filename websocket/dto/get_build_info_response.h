#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct get_build_info_response
{
    int32_t status;
    std::string date;
    std::vector<fs_entry> fs_entries;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_build_info_response>()
                .add_field(
                        CT_STRING("status"),
                        ptr_v<&get_build_info_response::status>{})
                .add_field(
                        CT_STRING("date"),
                        ptr_v<&get_build_info_response::date>{})
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&get_build_info_response::fs_entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
