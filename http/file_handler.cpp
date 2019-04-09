#include "file_handler.h"

#include "file_util.h"
#include "response.h"

namespace http
{

file_handler::file_handler(const std::string& doc_root)
    : doc_root_(doc_root)
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
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        std::string_view path)
{
        std::string full_path = path_cat(doc_root_, path);

        
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
        res.set(beast::http::field::content_type, mime_type(full_path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        queue.send(std::move(res));
        return handle_result::done;
}

} // namespace http
