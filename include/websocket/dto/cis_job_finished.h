/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_job_finished
{
    std::string status;
    int32_t exit_code;
    std::optional<std::string> session_id;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_finished>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("job"),
                        CT_STRING("finished"))
                .add_field(
                        CT_STRING("status"),
                        ptr_v<&cis_job_finished::status>{})
                .add_field(
                        CT_STRING("exit_code"),
                        ptr_v<&cis_job_finished::exit_code>{})
                .add_field(
                        CT_STRING("session_id"),
                        ptr_v<&cis_job_finished::session_id>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
