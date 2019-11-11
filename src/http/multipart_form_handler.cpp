#include "http/multipart_form_handler.h"

#include "cis/dirs.h"

namespace http
{

multipart_form_handler::multipart_form_handler(
        std::filesystem::path files_root,
        rights_manager& rights)
    : files_root_(std::move(files_root))
    , rights_(rights)
{}

handle_result multipart_form_handler::upload(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& /*queue*/,
        const std::string& project,
        const std::string& dir)
{
    if(req.method() == beast::http::verb::post
        && req[beast::http::field::content_type].find(
                "multipart/form-data") == 0)
    {
        if(auto project_rights
                = rights_.check_project_right(ctx.username, project);
                !((!project_rights)
                || (project_rights && project_rights.value().write)))
        {
            ctx.res_status = beast::http::status::forbidden;

            return handle_result::error;
        }

        reader.async_read_body<multipart_form_body>(
                [&](beast::http::request<multipart_form_body>& req)
                {
                    boost::beast::error_code ec;

                    req.body().set_dir(
                            (files_root_ / project / dir).string(),
                            ec);
                    if(ec)
                    {
                        req.body().set_mode(
                            multipart_form_body::value_type::mode::ignore);

                        return;
                    }

                    req.body().set_mode(
                            multipart_form_body::value_type::mode::add);
                },
                [&, ctx](
                    beast::http::request<multipart_form_body>&& req,
                    const boost::beast::error_code& ec,
                    net::http_session::queue& queue) mutable
                {
                    handle_body(std::move(req), ctx, ec, queue);
                });

        return handle_result::done;
    }

    ctx.res_status = beast::http::status::method_not_allowed;

    return handle_result::error;
}

handle_result multipart_form_handler::replace(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& /*queue*/,
        const std::string& project,
        const std::string& dir)
{
    if(req.method() == beast::http::verb::post
        && req[beast::http::field::content_type].find(
                "multipart/form-data") == 0)
    {
        if(auto project_rights
                = rights_.check_project_right(ctx.username, project);
                !((!project_rights)
                || (project_rights && project_rights.value().write)))
        {
            ctx.res_status = beast::http::status::forbidden;

            return handle_result::error;
        }

        reader.async_read_body<multipart_form_body>(
                [&](beast::http::request<multipart_form_body>& req)
                {
                    boost::beast::error_code ec;

                    req.body().set_dir(
                            (files_root_ / project / dir).string(),
                            ec);
                    if(ec)
                    {
                        req.body().set_mode(
                            multipart_form_body::value_type::mode::ignore);

                        return;
                    }

                    req.body().set_mode(
                            multipart_form_body::value_type::mode::replace);
                },
                [&, ctx](
                    beast::http::request<multipart_form_body>&& req,
                    const boost::beast::error_code& ec,
                    net::http_session::queue& queue) mutable
                {
                    handle_body(std::move(req), ctx, ec, queue);
                });

        return handle_result::done;
    }

    ctx.res_status = beast::http::status::method_not_allowed;

    return handle_result::error;
}


void multipart_form_handler::handle_body(
        beast::http::request<multipart_form_body>&& req,
        request_context& /*ctx*/,
        const boost::beast::error_code& ec,
        net::http_session::queue& queue)
{
    beast::http::status status = beast::http::status::ok;

    for(auto& [key, value] : req.body().get_values())
    {
        if(value.is_file && !value.written)
        {
            if(req.body().get_mode()
                    == multipart_form_body::value_type::mode::replace)
            {
                status = beast::http::status::not_found;
            }
            else if(req.body().get_mode()
                    == multipart_form_body::value_type::mode::add)
            {
                status = beast::http::status::conflict;
            }
        }
    }

    beast::http::response<beast::http::empty_body> res{
            status,
            req.version()};

    res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(beast::http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());

    queue.send(std::move(res));
}

} // namespace http
