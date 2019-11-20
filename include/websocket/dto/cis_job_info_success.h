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
    std::vector<
        std::variant<
            std::monostate,
            fs_entry>> properties;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_info_success>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("job"),
                        CT_STRING("info"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("fs_entries"),
                        ptr_v<&cis_job_info_success::fs_entries>{})
                .add_field(
                        CT_STRING("params"),
                        ptr_v<&cis_job_info_success::params>{})
                .add_field(
                        CT_STRING("properties"),
                        ptr_v<&cis_job_info_success::properties>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
