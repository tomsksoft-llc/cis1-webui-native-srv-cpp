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

struct admin_user
{
    std::string name;
    std::string email;
    std::string pass;
};

struct init_params
{
    std::string public_address;
    unsigned short public_port;
    std::string cis_address;
    unsigned short cis_port;
    std::filesystem::path doc_root;
    std::filesystem::path cis_root;
    std::filesystem::path db_root;
    std::optional<admin_user> admin;
};

std::optional<init_params> parse_args(
        int argc,
        char* argv[],
        std::error_code& ec);
