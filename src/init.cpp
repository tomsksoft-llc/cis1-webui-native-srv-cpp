/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "init.h"

#include <iostream>
#include <filesystem>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "cis/dirs.h"
#include "error_code.h"

namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace pt = boost::property_tree;

std::optional<init_params> parse_args(
        int argc,
        char* argv[],
        std::error_code& ec)
{
    init_params result{};

    if(argc == 2)
    {
        pt::ptree pt;
        try
        {
            pt::ini_parser::read_ini(argv[1], pt);
        }
        catch(pt::ini_parser_error& err)
        {
            ec = cis::error_code::cant_parse_config_ini;

            return std::nullopt;
        }

        auto opt_working_dir = pt.get_optional<std::string>(
                "global.working_dir");

        if(opt_working_dir)
        {
            std::filesystem::current_path(opt_working_dir.value(), ec);

            if(ec)
            {
                return std::nullopt;
            }
        }

        auto public_address_opt = pt.get_optional<std::string>("http.host");
        if(!public_address_opt)
        {
            ec = cis::error_code::cant_parse_config_ini;

            return std::nullopt;
        }
        result.public_address = public_address_opt.value();

        auto public_port_opt = pt.get_optional<unsigned short>("http.port");
        if(!public_port_opt)
        {
            ec = cis::error_code::cant_parse_config_ini;

            return std::nullopt;
        }
        result.public_port = public_port_opt.value();

        auto doc_root_opt = pt.get_optional<std::string>("http.doc_root");
        if(!doc_root_opt)
        {
            ec = cis::error_code::cant_parse_config_ini;

            return std::nullopt;
        }
        result.doc_root = doc_root_opt.value();


        auto cis_root_opt = pt.get_optional<std::string>("cis.cis_root");
        if(cis_root_opt)
        {
            result.cis_root = cis_root_opt.value();
        }
        else if(const char* cis_root = std::getenv("cis_base_dir");
                            cis_root != nullptr)
        {
            result.cis_root = cis_root;
        }
        else
        {
            ec = cis::error_code::incorrect_environment;

            return std::nullopt;
        }

        auto cis_address_opt = pt.get_optional<std::string>("cis.host");
        if(!cis_address_opt)
        {
            ec = cis::error_code::cant_parse_config_ini;

            return std::nullopt;
        }
        result.cis_address = cis_address_opt.value();

        auto cis_port_opt = pt.get_optional<unsigned short>("cis.port");
        if(!cis_port_opt)
        {
            ec = cis::error_code::cant_parse_config_ini;

            return std::nullopt;
        }
        result.cis_port = cis_port_opt.value();

        auto db_root_opt = pt.get_optional<std::string>("db.db_root");
        if(!db_root_opt)
        {
            ec = cis::error_code::cant_parse_config_ini;

            return std::nullopt;
        }
        result.db_root = db_root_opt.value();
    }
    else
    {
        result.public_address = "127.0.0.1";
        result.public_port = static_cast<unsigned short>(8080);
        result.cis_address = "127.0.0.1";
        result.cis_port = static_cast<unsigned short>(8081);
        result.doc_root = ".";

        if(const char* cis_base_dir = std::getenv("cis_base_dir");
                cis_base_dir != nullptr)
        {
            result.cis_root = cis_base_dir;
        }
        else
        {
            ec = cis::error_code::incorrect_environment;

            return std::nullopt;
        }

        result.db_root = ".";
    }

    cis::set_root_dir(result.cis_root.string().c_str());

    if(!std::filesystem::exists("webhooks_temp", ec))
    {
        std::filesystem::create_directory("webhooks_temp", ec);
    }

    if(!std::filesystem::exists(result.db_root / "db.sqlite", ec))
    {
        result.admin = admin_user{"admin", "admin@example.com" , "1234"};
    }

    return result;
}
