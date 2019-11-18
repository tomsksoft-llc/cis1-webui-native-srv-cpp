/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <vector>
#include <variant>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct fs_entry
{
    struct project_info
    {
        bool permitted;

        static constexpr auto get_name()
        {
            return CT_STRING("project");
        }

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<project_info>()
                    .add_field(
                            CT_STRING("permitted"),
                            ptr_v<&project_info::permitted>{})
                    .done();
        }
    };

    struct job_info
    {
        static constexpr auto get_name()
        {
            return CT_STRING("job");
        }

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<job_info>()
                    .done();
        }
    };

    struct build_info
    {
        std::optional<int> status;
        std::optional<std::string> date;

        static constexpr auto get_name()
        {
            return CT_STRING("build");
        }

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<build_info>()
                    .add_field(
                            CT_STRING("status"),
                            ptr_v<&build_info::status>{})
                    .add_field(
                            CT_STRING("date"),
                            ptr_v<&build_info::date>{})
                    .done();
        }
    };

    std::string name;
    bool binary;
    bool directory;
    std::string path;
    std::string link;
    std::variant<std::monostate, project_info, job_info, build_info> metainfo;

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
                .add_field(
                        CT_STRING("metainfo"),
                        ptr_v<&fs_entry::metainfo>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
