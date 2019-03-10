#pragma once

#include <string>
#include <boost/asio/ip/address.hpp>

struct init_params
{
    boost::asio::ip::address public_address;
    unsigned short public_port;
    boost::asio::ip::address cis_address;
    unsigned short cis_port;
    std::string doc_root;
    std::string cis_root;
};

init_params parse_args(int argc, char* argv[]);
