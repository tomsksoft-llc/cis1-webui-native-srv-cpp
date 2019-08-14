#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct auth_error_wrong_credentials 
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<auth_error_wrong_credentials>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
