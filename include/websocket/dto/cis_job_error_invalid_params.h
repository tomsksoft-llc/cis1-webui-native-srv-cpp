#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_job_error_invalid_params
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_error_invalid_params>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("job"),
                        CT_STRING("error"),
                        CT_STRING("invalid_params"))
                .done();
    }
};

} // namespace dto

} // nam
