#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct auth_token_response
{
    std::string group;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_token_response>()
                .add_field(
                        CT_STRING("group"),
                        ptr_v<&auth_token_response::group>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
