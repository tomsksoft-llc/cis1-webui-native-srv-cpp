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

struct user_list
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_list>()
                .set_name(
                        CT_STRING("user"),
                        CT_STRING("list"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
