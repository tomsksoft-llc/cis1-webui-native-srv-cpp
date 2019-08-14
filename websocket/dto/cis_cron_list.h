#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_cron_list
{
    std::string mask;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_cron_list>()
                .add_field(
                        CT_STRING("mask"),
                        ptr_v<&cis_cron_list::mask>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
