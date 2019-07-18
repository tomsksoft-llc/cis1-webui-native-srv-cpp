#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct add_cis_cron_request
{
    std::string project;
    std::string job;
    std::string cron_expr;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<add_cis_cron_request>()
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&add_cis_cron_request::project>{})
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&add_cis_cron_request::job>{})
                .add_field(
                        CT_STRING("cron_expr"),
                        ptr_v<&add_cis_cron_request::cron_expr>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
