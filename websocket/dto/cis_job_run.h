#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_job_run
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

    std::string project;
    std::string job;
    std::vector<param> params;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_run>()
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_job_run::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_job_run::job>{})
                .add_field(
                        CT_STRING("params"),
                        ptr_v<&cis_job_run::params>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
