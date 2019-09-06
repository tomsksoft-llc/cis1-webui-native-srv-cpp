#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct cis_job_info_success
{
    struct param
    {
        std::string name;
        std::string value;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<param>()
                    .add_field(
                            CT_STRING("name"),
                            ptr_v<&param::name>{})
                    .add_field(
                            CT_STRING("value"),
                            ptr_v<&param::value>{})
                    .done();
        }
    };

    std::vector<fs_entry> fs_entries;
    std::vector<param> params;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_info_success>()
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&cis_job_info_success::fs_entries>{})
                .add_field(
                        CT_STRING("params"),
                        ptr_v<&cis_job_info_success::params>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
