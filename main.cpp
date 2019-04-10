#include "init.h"
#include "application.h"

int main(int argc, char* argv[])
{
    auto params = parse_args(argc, argv);
    
    application app(params);
    app.run();

    return EXIT_SUCCESS;
}
