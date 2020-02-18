/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"
#include "path_utils.h"

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
                        ptr_v<&cis_job_add::project>{},
                        [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_job_add::job>{},
                        [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                .done();
    }
};

} // namespace dto

} // namespace websocket
