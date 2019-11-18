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

struct cis_cron_remove
{
    std::string project;
    std::string job;
    std::string cron_expr;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_cron_remove>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("cron"),
                        CT_STRING("remove"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_cron_remove::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_cron_remove::job>{})
                .add_field(
                        CT_STRING("cron_expr"),
                        ptr_v<&cis_cron_remove::cron_expr>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
