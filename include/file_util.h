/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <filesystem>

#include <boost/beast.hpp>

boost::beast::string_view mime_type(
        const std::filesystem::path &path);

std::string path_cat(
        boost::beast::string_view base,
        boost::beast::string_view path);
