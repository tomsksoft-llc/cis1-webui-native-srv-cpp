#pragma once

#include <string>

#include <boost/beast.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>

beast::string_view mime_type(beast::string_view path);

std::string path_cat(
    beast::string_view base,
    beast::string_view path);
