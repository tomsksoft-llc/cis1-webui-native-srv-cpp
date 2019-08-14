#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_project_list_get
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_project_list_get>()
                .done();
    }
};

} // namespace dto

} // namespace websocket
