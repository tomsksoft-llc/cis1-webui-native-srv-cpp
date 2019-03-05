#include "projects_handler.h"

#include "http_session.h"
#include "router.h"
#include "file_util.h"
#include "cis_util.h"

const char* CIS_JOBS_FOLDER = "/jobs";

projects_handler::projects_handler(const std::string& cis_root)
    : cis_jobs_(path_cat(cis_root, CIS_JOBS_FOLDER))
{}

void projects_handler::handle(
            http::request<http::string_body>&& req,
            http_session::queue& queue)
{
        beast::error_code ec;
        job_list jobs(cis_jobs_);
        jobs.fetch();
        // Handle an unknown error
        if(ec)
        {
            return queue.send(handlers::server_error(std::move(req), ec.message()));
        }
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.body() = jobs.to_string();
        res.prepare_payload();
        res.keep_alive(req.keep_alive());
        return queue.send(std::move(res));
}
