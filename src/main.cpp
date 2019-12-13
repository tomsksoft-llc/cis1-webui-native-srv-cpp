/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "init.h"
#include "application.h"

#ifdef __linux__
#include <signal.h>
#endif

int main(int argc, char* argv[])
{

#ifdef __linux__
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
#endif

    std::error_code ec;

    auto config = std::make_unique<configuration_manager>();

    parse_args(argc, argv, *config, ec);
    if(ec)
    {
        std::cout << "Can't parse params: "
                  << ec.message() << std::endl;

        return EXIT_FAILURE;
    }

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
	}

    return EXIT_SUCCESS;
}
