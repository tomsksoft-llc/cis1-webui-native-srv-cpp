#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct get_project_list_request
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<get_project_list_request>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
