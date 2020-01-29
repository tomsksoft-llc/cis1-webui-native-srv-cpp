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

action parse_args(
        int argc,
        char* argv[],
        configuration_manager& config,
        std::error_code& ec)
{
    switch(argc)
    {
        case 2:
        {
            if(std::string{"--version"} == argv[1])
            {
                return action::version;
            }
            else
            {
                pt::ptree pt;
                try
                {
                    pt::ini_parser::read_ini(argv[1], pt);
                }
                catch(pt::ini_parser_error& err)
                {
                    ec = cis::error_code::cant_parse_config_ini;

                    return action::error;
                }

                auto opt_working_dir = pt.get_optional<std::string>(
                        "global.working_dir");

                if(opt_working_dir)
                {
                    std::filesystem::current_path(opt_working_dir.value(), ec);

                    if(ec)
                    {
                        return action::error;
                    }
                }

                auto public_address_opt = pt.get_optional<std::string>("http.host");
                if(!public_address_opt)
                {
                    ec = cis::error_code::cant_parse_config_ini;

                    return action::error;
                }
                config.add_entry("public_address", public_address_opt.value());

                auto public_port_opt = pt.get_optional<unsigned short>("http.port");
                if(!public_port_opt)
                {
                    ec = cis::error_code::cant_parse_config_ini;

                    return action::error;
                }
                config.add_entry("public_port", public_port_opt.value());

                auto doc_root_opt = pt.get_optional<std::string>("http.doc_root");
                if(!doc_root_opt)
                {
                    ec = cis::error_code::cant_parse_config_ini;

                    return action::error;
                }
                config.add_entry("doc_root", std::filesystem::path{doc_root_opt.value()});

                auto cis_root_opt = pt.get_optional<std::string>("cis.cis_root");
                if(cis_root_opt)
                {
                    config.add_entry("cis_root", std::filesystem::path{cis_root_opt.value()});
                }
                else if(const char* cis_root = std::getenv("cis_base_dir");
                                    cis_root != nullptr)
                {
                    config.add_entry("cis_root", std::filesystem::path{cis_root});
                }
                else
                {
                    ec = cis::error_code::incorrect_environment;

                    return action::error;
                }

                auto cis_address_opt = pt.get_optional<std::string>("cis.host");
                if(!cis_address_opt)
                {
                    ec = cis::error_code::cant_parse_config_ini;

                    return action::error;
                }

                config.add_entry("cis_address", cis_address_opt.value());

                auto cis_port_opt = pt.get_optional<unsigned short>("cis.port");
                if(!cis_port_opt)
                {
                    ec = cis::error_code::cant_parse_config_ini;

                    return action::error;
                }

                config.add_entry("cis_port", cis_port_opt.value());

                auto db_root_opt = pt.get_optional<std::string>("db.db_root");
                if(!db_root_opt)
                {
                    ec = cis::error_code::cant_parse_config_ini;

                    return action::error;
                }

                config.add_entry("db_root", std::filesystem::path{db_root_opt.value()});

                break;
            }
        }
        case 1:
        {
            config.add_entry("public_address", std::string{"127.0.0.1"});
            config.add_entry("public_port", static_cast<unsigned short>(8080));
            config.add_entry("cis_address", std::string{"127.0.0.1"});
            config.add_entry("cis_port", static_cast<unsigned short>(8081));
            config.add_entry("doc_root", std::filesystem::path{"."});

            if(const char* cis_base_dir = std::getenv("cis_base_dir");
                    cis_base_dir != nullptr)
            {
                config.add_entry("cis_root", std::filesystem::path{cis_base_dir});
            }
            else
            {
                ec = cis::error_code::incorrect_environment;

                return action::error;
            }

            config.add_entry("db_root", std::filesystem::path{"."});

            break;
        }
        default:
        {
            ec = cis::error_code::too_many_args;

            return action::error;
        }
    }

    auto* cis_root = config.get_entry<std::filesystem::path>("cis_root", ec);

    cis::set_root_dir(cis_root->generic_string().c_str());

    if(!std::filesystem::exists("webhooks_temp", ec))
    {
        std::filesystem::create_directory("webhooks_temp", ec);
    }

    auto* db_root = config.get_entry<std::filesystem::path>("db_root", ec);

    if(!std::filesystem::exists(*db_root / "db.sqlite", ec))
    {
        config.add_entry(
                "admin_credentials",
                user_credentials{"admin", "admin@example.com", "1234"});
        config.add_entry(
                "guest_credentials",
                user_credentials{"guest", "" , ""});
    }

    return action::run;

}
