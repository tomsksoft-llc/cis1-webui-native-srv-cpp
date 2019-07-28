#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct add_cis_cron_response
{
    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<add_cis_cron_response>()
                .done();
    }
};

} // namespace dto

} // namespace websocket