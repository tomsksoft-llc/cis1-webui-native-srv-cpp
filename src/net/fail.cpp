/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "net/fail.h"

#include <iostream>

void fail(boost::beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}
