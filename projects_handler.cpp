#include "projects_handler.h"

#include "net/http_session.h"
#include "file_util.h"
#include "dirs.h"
#include "response.h"

projects_handler::projects_handler()
    : projects_(path_cat(cis::get_root_dir(), cis::PROJECTS))
{
    projects_.fetch();
}

web_app::handle_result projects_handler::operator()(
        web_app::request_t& req,
        web_app::queue_t& queue,
        web_app::context_t& ctx)
{
        beast::error_code ec;
        // Handle an unknown error
        if(ec)
        {
            queue.send(response::server_error(std::move(req), ec.message()));
            return web_app::handle_result::done;
        }
        http::response<http::string_body> res{
            http::status::internal_server_error,
            req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.body() = projects_.to_json_string();
        res.prepare_payload();
        res.keep_alive(req.keep_alive());
        queue.send(std::move(res));
        return web_app::handle_result::done;
}

void projects_handler::run(
        boost::asio::io_context& ctx,
        const std::string& project,
        const std::string& job)
{
    ::run_job(ctx, project, job);
}  

void projects_handler::update()
{
    projects_.fetch();
}
