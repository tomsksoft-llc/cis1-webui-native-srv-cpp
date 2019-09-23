#pragma once

#include <boost/beast/core/file.hpp>

#include "basic_multipart_form_body.h"

using multipart_form_body = basic_multipart_form_body<boost::beast::file>;
