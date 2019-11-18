/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <boost/beast/core/error.hpp>   // for error_code

void fail(boost::beast::error_code ec, char const* what);
