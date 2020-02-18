/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <vector>

#include "tpl_reflect/meta_converter.h"
#include "path_utils.h"

namespace websocket
{

namespace dto
{

struct cis_job_run
{
    struct param
    {
        std::string name;
        std::string value;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<param>()
                    .add_field(
                            CT_STRING("name"),
                            ptr_v<&param::name>{})
                    .add_field(
                            CT_STRING("value"),
                            ptr_v<&param::value>{})
                    .done();
        }

    };

    std::string project;
    std::string job;
    bool force;
    std::vector<param> params;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_job_run>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("job"),
                        CT_STRING("run"))
                .add_field(
                        CT_STRING("project"),
                        ptr_v<&cis_job_run::project>{},
                        [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                .add_field(
                        CT_STRING("job"),
                        ptr_v<&cis_job_run::job>{},
                        [](auto&&... args){ return validate_path_fragment(std::forward<decltype(args)>(args)...); })
                .add_field(
                        CT_STRING("force"),
                        ptr_v<&cis_job_run::force>{})
                .add_field(
                        CT_STRING("params"),
                        ptr_v<&cis_job_run::params>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
