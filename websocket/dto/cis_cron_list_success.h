#pragma once

#include <vector>
#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_cron_list_success
{
    struct entry
    {
        std::string project;
        std::string job;
        std::string cron_expr;

        static constexpr auto get_converter()
        {
            using namespace reflect;
            return make_meta_converter<entry>()
                    .add_field(
                            CT_STRING("project"),
                            ptr_v<&entry::project>{})
                    .add_field(
                            CT_STRING("job"),
                            ptr_v<&entry::job>{})
                    .add_field(
                            CT_STRING("cron_expr"),
                            ptr_v<&entry::cron_expr>{})
                    .done();
        }
    };

    std::vector<entry> entries;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_cron_list_success>()
                .add_field(
                        CT_STRING("entries"),
                        ptr_v<&cis_cron_list_success::entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
