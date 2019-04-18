#pragma once

#include <string>
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
    std::string doc_root;
    std::string cis_root;
    std::string db_root;
    std::optional<admin_user> admin;
};

init_params parse_args(int argc, char* argv[]);
