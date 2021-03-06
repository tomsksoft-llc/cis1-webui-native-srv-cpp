/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "http/file_handler.h"

#include "file_util.h"

namespace http
{

file_handler::file_handler(std::string doc_root, bool download /*=false*/)
    : doc_root_(std::move(doc_root))
    , download_(download)
{}

handle_result file_handler::operator()(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue)
{
    return single_file(req, ctx, reader, queue, req.target());
}

handle_result file_handler::single_file(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& /*reader*/,
        net::http_session::queue& queue,
        std::string_view path)
{
    if(req.method() == beast::http::verb::get)
    {
        std::string full_path = path_cat(doc_root_, path);
        std::filesystem::path file_path(full_path);

        beast::error_code ec;
        beast::http::file_body::value_type body;
        body.open(full_path.c_str(), beast::file_mode::scan, ec);

        if(ec == beast::errc::no_such_file_or_directory)
        {
            ctx.res_status = beast::http::status::not_found;
            return handle_result::error;
        }

        // Handle an unknown error
        if(ec)
        {
            ctx.res_status = beast::http::status::internal_server_error;
            ctx.error = ec.message();
            return handle_result::error;
        }

        // Cache the size since we need it after the move
        auto const size = body.size();
        // Respond to GET request
        beast::http::response<beast::http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(beast::http::status::ok, req.version())};

        res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);

        res.set(beast::http::field::content_type, mime_type(file_path));

        if(download_)
        {
            std::stringstream ss;
            ss << "attachment; filename=" << file_path.filename();
            res.set(beast::http::field::content_disposition, ss.str());
        }
        else
        {
            res.set(beast::http::field::content_disposition, "inline");
        }

        res.content_length(size);
        res.keep_alive(req.keep_alive());

        queue.send(std::move(res));

        return handle_result::done;
    }

    ctx.allowed_verbs = {
        beast::http::verb::get
    };
    ctx.res_status = beast::http::status::method_not_allowed;

    return handle_result::error;
}

handle_result file_handler::sef(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue)
{
    auto real_path = std::string(req.target()) + ".html";

    return single_file(req, ctx, reader, queue, real_path);
}

} // namespace http
