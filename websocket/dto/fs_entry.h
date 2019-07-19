#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct fs_entry
{
    std::string name;
    bool binary;
    bool directory;
    std::string path;
    std::string link;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry>()
                .add_field(
                        CT_STRING("name"),
                        ptr_v<&fs_entry::name>{})
                .add_field(
                        CT_STRING("binary"),
                        ptr_v<&fs_entry::binary>{})
                .add_field(
                        CT_STRING("directory"),
                        ptr_v<&fs_entry::directory>{})
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&fs_entry::path>{})
                .add_field(
                        CT_STRING("link"),
                        ptr_v<&fs_entry::link>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
