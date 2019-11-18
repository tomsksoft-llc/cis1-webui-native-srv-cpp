/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_cron_add_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_cron_add_success>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("cron"),
                        CT_STRING("add"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
