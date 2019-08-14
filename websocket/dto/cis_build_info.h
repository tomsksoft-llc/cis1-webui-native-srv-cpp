#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

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
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_build_info::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_build_info::job>{})
                .add_field(
                        CT_STRING("build"),
                        ptr_v<&cis_build_info::build>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
