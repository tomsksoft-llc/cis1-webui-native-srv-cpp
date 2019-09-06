#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_job_error_doesnt_exist
{
    std::string project;
    std::string job;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_error_doesnt_exist>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("job"),
                        CT_STRING("error"),
                        CT_STRING("doesnt_exist"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_job_error_doesnt_exist::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_job_error_doesnt_exist::job>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
