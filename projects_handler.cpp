#include "projects_handler.h"

#include "http_session.h"
#include "router.h"
#include "file_util.h"

const char* CIS_PROJECTS_FOLDER = "/jobs";

projects_handler::projects_handler(const std::string& cis_root)
    : projects_(path_cat(cis_root, CIS_PROJECTS_FOLDER))
{
    projects_.fetch();
}

void projects_handler::get_projects(
            http::request<http::string_body>&& req,
            http_session::queue& queue)
{
        beast::error_code ec;
        // Handle an unknown error
        if(ec)
        {
            return queue.send(handlers::server_error(std::move(req), ec.message()));
        }
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.body() = projects_.to_json_string();
        res.prepare_payload();
        res.keep_alive(req.keep_alive());
        return queue.send(std::move(res));
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
