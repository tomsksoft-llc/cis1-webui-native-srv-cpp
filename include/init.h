/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <optional>
#include <filesystem>

#include <boost/asio/ip/address.hpp>

#include "configuration_manager.h"

struct user_credentials
{
    std::string email;
    std::string pass;
};

enum class action
{
    run,
    version,
    error,
};

action parse_args(
        int argc,
        char* argv[],
        configuration_manager& config,
        std::error_code& ec);
