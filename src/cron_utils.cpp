#include "cron_utils.h"

#include <regex>

namespace cron
{

static const std::regex cron_regex(
        R"rx(^(\*|([0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|5[0-9]))rx"
        R"rx(|\*\/([0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|5[0-9])) )rx"
        R"rx((\*|([0-9]|1[0-9]|2[0-3])|\*\/([0-9]|1[0-9]|2[0-3])) )rx"
        R"rx((\*|([1-9]|1[0-9]|2[0-9]|3[0-1])|\*\/([1-9]|1[0-9]|2[0-9]|3[0-1])) )rx"
        R"rx((\*|([1-9]|1[0-2])|\*\/([1-9]|1[0-2])) )rx"
        R"rx((\*|([0-6])|\*\/([0-6]))$)rx");

bool validate_expr(const std::string& expr)
{
    return std::regex_match(expr, cron_regex);
}

} // namespace cron
