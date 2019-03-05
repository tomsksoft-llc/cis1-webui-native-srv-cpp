#include "file_handler.h"

#include "http_session.h"
#include "router.h"
#include "file_util.h"

file_handler::file_handler(const std::string& doc_root)
    : doc_root_(doc_root)
{}

void file_handler::handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue,
            const std::string only_path)
{
        std::string path = 
            only_path.empty() ? 
            path_cat(doc_root_, req.target()) :
            path_cat(doc_root_, only_path);

        
        beast::error_code ec;
        http::file_body::value_type body;
        body.open(path.c_str(), beast::file_mode::scan, ec);
        if(ec == beast::errc::no_such_file_or_directory)
        {
            return queue.send(handlers::not_found(std::move(req)));
        }

        // Handle an unknown error
        if(ec)
        {
            return queue.send(handlers::server_error(std::move(req), ec.message()));
        }

        // Cache the size since we need it after the move
        auto const size = body.size();
        // Respond to GET request
        http::response<http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return queue.send(std::move(res));
}
