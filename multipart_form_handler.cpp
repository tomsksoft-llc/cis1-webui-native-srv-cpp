#include "multipart_form_handler.h"

#include "response.h"
#include "beast_ext/multipart_form_body.h"

handle_result multipart_form_handler::operator()(
        http::request<http::empty_body>& req,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        request_context& ctx,
        const std::string& save_dir)
{
    if(req.method() == http::verb::post
            && req[http::field::content_type].find("multipart/form-data") == 0)
    {
        std::string boundary;
        auto boundary_begin = req[http::field::content_type].find("=");
        if(boundary_begin != req[http::field::content_type].npos)
        {
            boundary = req[http::field::content_type].substr(
                    boundary_begin + 1,
                    req[http::field::content_type].size());
        }
        reader.async_read_body<multipart_form_body>(
                [&boundary, save_dir](http::request<multipart_form_body>& req)
                {
                    boost::beast::error_code ec;
                    req.body().set_boundary(boundary);
                    req.body().set_dir(save_dir, ec);
                },
                [ctx](
                    http::request<multipart_form_body>&& req,
                    net::http_session::queue& queue)
                {
                    http::response<http::empty_body> res{
                        http::status::ok,
                        req.version()};
                        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                        res.set(http::field::content_type, "text/html");
                        res.keep_alive(req.keep_alive());
                    queue.send(std::move(res));
                });
    }
    else
    {
        queue.send(response::not_found(std::move(req)));
        reader.done();
    }
    return handle_result::done;
}
