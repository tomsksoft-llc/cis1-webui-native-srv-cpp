/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "init.h"
#include "application.h"
#include "version.h"

void print_version()
{
    std::cout << cis_current_version;

    if(git_retreived_state)
    {
        std::cout << " " << git_sha1
                  << (git_is_dirty ? " dirty" : "") << std::endl;
    }
}

int main(int argc, char* argv[])
{
    std::error_code ec;

    auto config = std::make_unique<configuration_manager>();

    if(git_retreived_state)
    {
        config->add_entry("git/sha1", std::string{git_sha1});
        config->add_entry("git/dirty", git_is_dirty);
    }

    config->add_entry("cis/version", std::string{cis_current_version});

    auto act = parse_args(argc, argv, *config, ec);
    if(ec)
    {
        std::cout << ec.message() << std::endl;

        return EXIT_FAILURE;
    }

    switch(act)
    {
        case action::run:
        {
            std::cout << "Starting CIS1 WebUI..." << std::endl;

            std::cout << "Version: ";

            print_version();

            boost::asio::io_context ioc;

            boost::asio::signal_set signals(
                    ioc,
                    SIGINT,
                    SIGTERM);

            signals.async_wait(
                    [&ioc]( const beast::error_code& /*error*/,
                            int /*signal*/)
                    {
                        ioc.stop();
                    });

            auto app_opt = application::create(ioc, std::move(config), ec);
            if(ec)
            {
                std::cout << "Can't create WebUI instance: "
                          << ec.message() << std::endl;

                return EXIT_FAILURE;
            }
            auto& app = app_opt.value();

            try
            {
                app.run();
            }
            catch(...)
            {
                std::cout << "App crashed due to unhandled exception." << std::endl;

                return EXIT_FAILURE;
            }

            return EXIT_SUCCESS;
        }
        case action::version:
        {
            print_version();

            return EXIT_SUCCESS;
        }
        default:
        {
            std::cout << "Bad programm state." << std::endl;

            return EXIT_FAILURE;
        }
    }
}
