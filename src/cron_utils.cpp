#include "cron_utils.h"

#include <croncpp.h>

namespace cron
{

bool validate_expr(const std::string& expr)
{
    try
    {
        cron::make_cron(expr);

        return true;
    }
    catch(const cron::bad_cronexpr&)
    {
        return false;
    }
}

} // namespace cron
