#pragma once

#include <boost/beast.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>

void fail(beast::error_code ec, char const* what);
