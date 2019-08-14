#pragma once

#include <vector>
#include <string>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct cis_project_info_success
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
        return make_meta_converter<cis_project_info_success>()
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&cis_project_info_success::fs_entries>{})
                .add_field(
                        CT_STRING("jobs"),
                        ptr_v<&cis_project_info_success::jobs>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
