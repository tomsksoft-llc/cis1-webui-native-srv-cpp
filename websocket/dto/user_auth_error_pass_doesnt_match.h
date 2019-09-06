#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_auth_error_pass_doesnt_match
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_auth_error_pass_doesnt_match>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
