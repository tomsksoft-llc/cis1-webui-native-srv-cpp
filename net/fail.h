#pragma once

#include <boost/beast/core/error.hpp>   // for error_code

void fail(boost::beast::error_code ec, char const* what);
