#pragma once

#include <string>

#include "tpl_reflect/meta_converter.h"

namespace websocket
{

namespace dto
{

struct cis_session_log_entry
{
    std::string session_id;
    std::chrono::time_point<std::chrono::system_clock> time;
    std::string message;

    static constexpr auto get_converter()
    {
        using namespace reflect;
        return make_meta_converter<cis_session_log_entry>()
                .set_name(
                        CT_STRING("cis"),
                        CT_STRING("session"),
                        CT_STRING("log_entry"))
                .add_field(
                        CT_STRING("session_id"),
                        ptr_v<&cis_session_log_entry::session_id>{})
                .add_field(
                        CT_STRING("time"),
                        ptr_v<&cis_session_log_entry::time>())
                .add_field(
                        CT_STRING("message"),
                        ptr_v<&cis_session_log_entry::message>{})
                .done();
    }
};

} // namespace dto

} // namespace websocket
