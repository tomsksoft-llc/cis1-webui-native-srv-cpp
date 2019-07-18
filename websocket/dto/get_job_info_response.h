#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "fs_entry.h"

namespace websocket
{

namespace dto
{

struct get_job_info_response
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

    struct build
    {
        std::string name;
        int32_t status;
        std::string date;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<build>()
                    .add_field(
                            CT_STRING("name"),
                            ptr_v<&build::name>{})
                    .add_field(
                            CT_STRING("status"),
                            ptr_v<&build::status>{})
                    .add_field(
                            CT_STRING("date"),
                            ptr_v<&build::date>{})
                    .done();
        }
    };

    std::vector<fs_entry> fs_entries;
    std::vector<param> params;
    std::vector<build> builds;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_job_info_response>()
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&get_job_info_response::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&get_job_info_response::job>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
