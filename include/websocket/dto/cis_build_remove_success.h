#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_build_remove_success
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_build_remove_success>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("build"),
                        CT_STRING("remove"),
                        CT_STRING("success"))
                .done();
    }
};

} // namespace dto

} // namespace websocket
