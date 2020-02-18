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

struct cis_build_info
{
    std::string project;
    std::string job;
    std::string build;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_build_info>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("build"),
                        CT_STRING("info"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_build_info::project>{},
                        [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_build_info::job>{},
                        [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                .add_field(
                        CT_STRING("build"),
                        ptr_v<&cis_build_info::build>{},
                        [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                .done();
    }
};

} // namespace dto

} // namespace websocket
