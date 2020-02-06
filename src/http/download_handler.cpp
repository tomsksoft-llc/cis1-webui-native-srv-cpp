/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "http/download_handler.h"

namespace http
{

download_handler::download_handler(
        std::filesystem::path files_root,
        rights_manager& rights)
    : rights_(rights)
    , files_(std::move(files_root).string(), true)
{}

handle_result download_handler::operator()(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        const std::string& project,
        const std::string& file)
{
    if(req.method() == beast::http::verb::get)
    {
        std::error_code ec;

        auto project_rights
                = rights_.check_project_right(ctx.client_info, project, ec);

        if(ec)
        {
            ctx.res_status = beast::http::status::internal_server_error;

            return handle_result::error;
        }

        if(!project_rights || !project_rights.value().write)
        {
            ctx.res_status = beast::http::status::forbidden;

            return handle_result::error;
        }

        std::filesystem::path p{"/"};

        p = p / project / file;

        return files_.single_file(req, ctx, reader, queue, p.string());
    }

    ctx.res_status = beast::http::status::not_found;

    return handle_result::error;
}

} // namespace http
