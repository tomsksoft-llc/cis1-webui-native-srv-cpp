#pragma once

#include <vector>
#include <string>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct get_project_info_response
{
    struct job
    {
        std::string name;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<job>()
                    .add_field(
                            CT_STRING("name"),
                            ptr_v<&job::name>{})
                    .done();
        }
    };

    std::vector<fs_entry> fs_entries;
    std::vector<job> jobs;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_project_info_response>()
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&get_fs_entries_info_response::fs_entries>{})
                .add_field(
                        CT_STRING("jobs"),
                        ptr_v<&get_jobs_info_response::jobs>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
