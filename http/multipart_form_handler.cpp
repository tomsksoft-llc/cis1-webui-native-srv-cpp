#include "multipart_form_handler.h"

#include "response.h"
#include "beast_ext/multipart_form_body.h"

namespace http
{

handle_result multipart_form_handler::operator()(
        beast::http::request<beast::http::empty_body>& req,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        request_context& ctx,
        const std::string& save_dir)
{
    if(req.method() == beast::http::verb::post
            && req[beast::http::field::content_type].find("multipart/form-data") == 0)
    {
        std::string boundary;
        auto boundary_begin = req[beast::http::field::content_type].find("=");
        if(boundary_begin != req[beast::http::field::content_type].npos)
        {
            boundary = req[beast::http::field::content_type].substr(
                    boundary_begin + 1,
                    req[beast::http::field::content_type].size());
        }
        reader.async_read_body<multipart_form_body>(
                [&boundary, save_dir](beast::http::request<multipart_form_body>& req)
                {
                    boost::beast::error_code ec;
                    req.body().set_boundary(boundary);
                    req.body().set_dir(save_dir, ec);
                },
                [ctx](
                    beast::http::request<multipart_form_body>&& req,
                    net::http_session::queue& queue)
                {
                    beast::http::response<beast::http::empty_body> res{
                        beast::http::status::ok,
                        req.version()};
                        res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                        res.set(beast::http::field::content_type, "text/html");
                        res.keep_alive(req.keep_alive());
                    queue.send(std::move(res));
                });
        return handle_result::done;
    }
    ctx.res_status = beast::http::status::not_found;
    return handle_result::error;
    reader.done();
}

} // namespace http
