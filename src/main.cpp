/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "init.h"
#include "application.h"

int main(int argc, char* argv[])
{
    std::error_code ec;

    auto params_opt = parse_args(argc, argv, ec);
    if(ec)
    {
        std::cout << "Can't parse params: "
                  << ec.message() << std::endl;

        return EXIT_FAILURE;
    }
    auto& params = params_opt.value();

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

    auto app_opt = application::create(ioc, params, ec);
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
