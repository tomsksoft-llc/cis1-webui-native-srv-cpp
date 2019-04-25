#pragma once

#include <vector>
#include <string>
#include <system_error>

#include <boost/asio.hpp>

namespace cis
{

void run_job(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& name,
        const std::vector<std::string>& params = {});

void rename_job(
        const std::string& project,
        const std::string& old_name,
        const std::string& new_name,
        std::error_code& ec);

} // namespace cis
