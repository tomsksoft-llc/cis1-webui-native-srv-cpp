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

struct system_version_info
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<system_version_info>()
                .set_name(
                        CT_STRING("system"),
                        CT_STRING("version"),
                        CT_STRING("info"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
