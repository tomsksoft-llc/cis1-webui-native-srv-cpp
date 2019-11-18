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

struct cis_session_unsubscribe
{
    std::string session_id;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_session_unsubscribe>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("session"),
                        CT_STRING("unsubscribe"))
                .add_field(
                        CT_STRING("session_id"),
                        ptr_v<&cis_session_unsubscribe::session_id>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
