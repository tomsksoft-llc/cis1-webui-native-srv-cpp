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
        std::optional<std::string> status;
        std::optional<int> exit_code;
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
                            CT_STRING("exit_code"),
                            ptr_v<&build_info::exit_code>{})
                    .add_field(
                            CT_STRING("date"),
                            ptr_v<&build_info::date>{})
                    .done();
        }
    };

    struct directory_info
    {
        static constexpr auto get_name()
        {
            return CT_STRING("directory");
        }

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<directory_info>()
                    .done();
        }
    };

    struct file_info
    {
        bool executable;

        static constexpr auto get_name()
        {
            return CT_STRING("file");
        }

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<file_info>()
                    .add_field(
                            CT_STRING("executable"),
                            ptr_v<&file_info::executable>{})
                    .done();
        }
    };

    std::string name;
    std::string path;
    std::string link;
    std::variant<
        std::monostate,
        project_info,
        job_info,
        build_info,
        directory_info,
        file_info> metainfo;

    static constexpr auto get_name()
    {
        return CT_STRING("fs_entry");
    }

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<fs_entry>()
                .add_field(
                        CT_STRING("name"),
                        ptr_v<&fs_entry::name>{})
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
