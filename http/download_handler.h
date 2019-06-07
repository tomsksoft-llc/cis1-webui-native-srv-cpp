#pragma once

#include <filesystem>

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"
#include "rights_manager.h"
#include "beast_ext/multipart_form_body.h"
#include "file_handler.h"

namespace beast = boost::beast;

namespace http
{

class download_handler 
{
    rights_manager& rights_;
    file_handler files_;
public:
    download_handler(
            std::filesystem::path files_root,
            rights_manager& rights);
    handle_result operator()(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& file);
};

} // namespace http
