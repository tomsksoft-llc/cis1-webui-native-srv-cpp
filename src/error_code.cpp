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

        case error_code::cant_parse_config_ini:
            return "Can't parse config.ini";

        case error_code::incorrect_environment:
            return "Incorrect environment";

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
