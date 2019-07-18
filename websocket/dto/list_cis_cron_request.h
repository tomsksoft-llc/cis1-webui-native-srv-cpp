#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct list_cis_cron_request
{
    std::string mask;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<list_cis_cron_request>()
                .add_field(
                        CT_STRING("mask"),
                        ptr_v<&list_cis_cron_request::mask>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
