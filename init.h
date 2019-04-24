#pragma once

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
    boost::asio::ip::address public_address;
    unsigned short public_port;
    boost::asio::ip::address cis_address;
    unsigned short cis_port;
    std::filesystem::path doc_root;
    std::filesystem::path cis_root;
    std::filesystem::path db_root;
    std::optional<admin_user> admin;
};

init_params parse_args(int argc, char* argv[]);
