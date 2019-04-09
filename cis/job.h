#pragma once

#include <string>
#include <system_error>

#include <boost/asio.hpp>

namespace cis
{

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name);

void rename_job(
        const std::string& project,
        const std::string& job,
        const std::string& name,
        std::error_code& ec);

} // namespace cis
