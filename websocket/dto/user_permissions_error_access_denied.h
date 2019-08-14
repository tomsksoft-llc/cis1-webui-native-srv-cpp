#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct user_permissions_error_access_denied
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<user_permissions_error_access_denied>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
