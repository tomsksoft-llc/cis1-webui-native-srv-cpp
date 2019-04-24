#pragma once

#include <string>

#include <boost/beast.hpp>

boost::beast::string_view mime_type(
        boost::beast::string_view path);

std::string path_cat(
        boost::beast::string_view base,
        boost::beast::string_view path);
