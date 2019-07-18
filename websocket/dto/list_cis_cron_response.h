#pragma once

#include <vector>
#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct list_cis_cron_response
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
        return make_meta_converter<list_cis_cron_response>()
                .add_field(
                        CT_STRING("entries"),
                        ptr_v<&list_cis_cron_response::entries>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
