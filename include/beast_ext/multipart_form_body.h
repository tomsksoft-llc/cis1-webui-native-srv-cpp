/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <boost/beast/core/file.hpp>

#include "basic_multipart_form_body.h"

using multipart_form_body = basic_multipart_form_body<boost::beast::file>;
