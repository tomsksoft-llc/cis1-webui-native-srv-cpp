#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct get_project_list_response
{
    struct project
    {
        std::string name;
        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<project>()
                .add_field(
                        CT_STRING("name"),
                        ptr_v<&project::name>{})
                .done();
        }
    };

    std::vector<fs_entry> fs_entries;
    std::vector<project> projects;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_project_list_response>()
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&get_project_list_response::fs_entries>{})
                .add_field(
                        CT_STRING("projects"),
                        ptr_v<&get_project_list_response::projects>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
