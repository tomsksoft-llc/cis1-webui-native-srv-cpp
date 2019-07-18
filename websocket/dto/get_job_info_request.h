#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct get_job_info_request
{
    std::string project;
    std::string job;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_job_info_request>()
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&get_job_info_request::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&get_job_info_request::job>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
