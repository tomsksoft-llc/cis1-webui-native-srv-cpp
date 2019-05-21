#include "init.h"

#include <iostream>
#include <filesystem>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "cis/dirs.h"

namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace pt = boost::property_tree;

init_params parse_args(int argc, char* argv[])
{
    init_params result{};

    if(argc == 2)
    {
        pt::ptree pt;
        pt::ini_parser::read_ini(argv[1], pt);
        auto opt_working_dir = pt.get_optional<std::string>("global.working_dir");

        if(opt_working_dir)
        {
            std::filesystem::current_path(opt_working_dir.value());
        }

        result.public_address = net::ip::make_address(
                pt.get<std::string>("http.ip"));
        result.public_port = pt.get<unsigned short>("http.port");
        result.doc_root = pt.get<std::string>("http.doc_root");
        auto opt_cis_root = pt.get_optional<std::string>("cis.cis_root");

        if(opt_cis_root)
        {
            result.cis_root = opt_cis_root.value();
        }
        else
        {
            result.cis_root = std::getenv("cis_base_dir");
        }

        result.cis_address = net::ip::make_address(pt.get<std::string>("cis.ip"));
        result.cis_port = pt.get<unsigned short>("cis.port");
        result.db_root = pt.get<std::string>("db.db_root");
    }
    else
    {
        result.public_address = net::ip::make_address("127.0.0.1");
        result.public_port = static_cast<unsigned short>(8080);
        result.cis_address = net::ip::make_address("127.0.0.1");
        result.cis_port = static_cast<unsigned short>(8081);
        result.doc_root = ".";

        if(const auto* cis_base_dir = std::getenv("cis_base_dir");
                cis_base_dir != nullptr)
        {
            result.cis_root = cis_base_dir;
        }
        else
        {
            result.cis_root = "./cis";
        }

        result.db_root = ".";
    }

    cis::set_root_dir(result.cis_root.c_str());

    if(!std::filesystem::exists("webhooks_temp"))
    {
        std::filesystem::create_directory("webhooks_temp");
    }

    if(!std::filesystem::exists(result.db_root / "db.sqlite"))
    {
        result.admin = admin_user{};
        std::cout << "Enter admin username: ";
        std::cin >> result.admin.value().name;
        std::cout << "Enter admin email: ";
        std::cin >> result.admin.value().email;
        std::cout << "Enter admin password: ";
        std::cin >> result.admin.value().pass;
    }

    return result;
}
