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

namespace websocket
{

namespace dto
{

struct cis_project_error_doesnt_exist
{
    std::string project;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_project_error_doesnt_exist>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("project"),
                        CT_STRING("error"),
                        CT_STRING("doesnt_exist"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_project_error_doesnt_exist::project>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
