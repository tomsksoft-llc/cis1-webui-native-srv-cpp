#include "init.h"
#include "application.h"

int main(int argc, char* argv[])
{
    auto params = parse_args(argc, argv);

	try
	{
        application app(params);
        app.run();
	}
	catch(std::system_error ec)
	{
        std::cout << "App crashed due to internal error." << std::endl;
	}

    return EXIT_SUCCESS;
}
