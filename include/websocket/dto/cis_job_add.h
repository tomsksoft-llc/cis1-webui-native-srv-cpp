#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_job_add
{
    std::string project;
    std::string job;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_add>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("job"),
                        CT_STRING("add"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_job_add::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_job_add::job>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
