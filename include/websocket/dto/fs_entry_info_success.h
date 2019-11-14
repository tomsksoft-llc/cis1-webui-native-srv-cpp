#pragma once

#include <string>
#include <vector>
#include <variant>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct fs_entry_info_success
{

    std::string name;
    bool binary;
    bool directory;
    std::string path;
    std::string link;
    std::variant<
        std::monostate,
        fs_entry::project_info,
        fs_entry::job_info,
        fs_entry::build_info> metainfo;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry_info_success>()
                .set_name(
                        CT_STRING("fs"),
                        CT_STRING("entry"),
                        CT_STRING("info"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("name"),
                        ptr_v<&fs_entry_info_success::name>{})
                .add_field(
                        CT_STRING("binary"),
                        ptr_v<&fs_entry_info_success::binary>{})
                .add_field(
                        CT_STRING("directory"),
                        ptr_v<&fs_entry_info_success::directory>{})
                .add_field(
                        CT_STRING("path"),
                        ptr_v<&fs_entry_info_success::path>{})
                .add_field(
                        CT_STRING("link"),
                        ptr_v<&fs_entry_info_success::link>{})
                .add_field(
                        CT_STRING("metainfo"),
                        ptr_v<&fs_entry_info_success::metainfo>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
