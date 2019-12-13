/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "error_code.h"

namespace cis
{

const char * error_category::name() const noexcept
{
    return "cis::error";
}

std::string error_category::message(int ev) const
{
    switch(static_cast<error_code>(ev))
    {
        case error_code::ok:
            return "OK";

        case error_code::too_many_args:
            return "Too many arguments";

        case error_code::cant_parse_config_ini:
            return "Can't parse config.ini";

        case error_code::incorrect_environment:
            return "Incorrect environment";

        case error_code::cant_run_http_listener:
            return "Can't run HTTP listener";

        case error_code::cant_resolve_address:
            return "Can't resolve network address";

        case error_code::cant_open_file:
            return "Can't open file";

        case error_code::cant_cast_config_entry:
            return "Can't cast config entry";

        case error_code::database_error:
            return "Database error";

        default:
            return "(unrecognized error)";
    }
}

const error_category category;

std::error_code make_error_code(error_code e)
{
    return {static_cast<int>(e), category};
}

} // namespace cis
