#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct logout_request
{
    std::string token;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<logout_request>()
                .add_field(
                        CT_STRING("token"),
                        ptr_v<&logout_request::token>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
