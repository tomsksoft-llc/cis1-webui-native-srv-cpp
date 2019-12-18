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

struct system_version_info_success
{
    std::string webui_version;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<system_version_info_success>()
                .set_name(
                        CT_STRING("system"),
                        CT_STRING("version"),
                        CT_STRING("info"),
                        CT_STRING("success"))
                .add_field(
                        CT_STRING("webui_version"),
                        ptr_v<&system_version_info_success::webui_version>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
