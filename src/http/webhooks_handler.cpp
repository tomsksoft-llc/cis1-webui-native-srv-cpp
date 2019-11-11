#include "http/webhooks_handler.h"

#include <sstream>
#include <iomanip>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "openssl_wrapper/openssl_wrapper.h"
#include "http/request_util.h"
#include "random_generator.h"

namespace http
{

webhooks_handler::webhooks_handler(
        auth_manager& auth,
        rights_manager& rights,
        cis::cis_manager& cis)
    : auth_(auth)
    , rights_(rights)
    , cis_(cis)
{}

handle_result webhooks_handler::operator()(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        const std::string& username,
        const std::string& project,
        const std::string& job,
        const std::string& escaped_query_string,
        api api_provider)
{
    auto user = auth_.get_user_info(username);

    if(!user)
    {
        ctx.res_status = beast::http::status::not_found;
        return http::handle_result::error;
    }

    if(!user->api_access_key)
    {
        ctx.res_status = beast::http::status::forbidden;
        return handle_result::error;
    }

    ctx.username = username;
    ctx.api_access_key = user->api_access_key.value();

    if(auto project_rights
            = rights_.check_project_right(ctx.username, project);
            !((!project_rights)
            || (project_rights && project_rights.value().write)))
    {
        ctx.res_status = beast::http::status::forbidden;
        ctx.error = "Forbidden.";
        return handle_result::error;
    }

    auto query_string = unescape_uri(escaped_query_string);

    switch(api_provider)
    {
        case api::github:
        {
            return handle_github_headers(
                    req,
                    ctx,
                    reader,
                    queue,
                    project,
                    job,
                    query_string);
        }
        case api::gitlab:
        {
            return handle_gitlab_headers(
                    req,
                    ctx,
                    reader,
                    queue,
                    project,
                    job,
                    query_string);
        }
        case api::plain:
        {
            return handle_plain_headers(
                    req,
                    ctx,
                    reader,
                    queue,
                    project,
                    job,
                    query_string);
        }
        default:
        {
            return handle_result::error;
        }
    }
}

handle_result webhooks_handler::handle_github_headers(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        const std::string& project,
        const std::string& job,
        const std::string& query_string)
{
    auto signature_it = req.find("X-Hub-Signature");
    auto event_it = req.find("X-GitHub-Event");
    if(signature_it == req.end()
            || event_it == req.end()
            || signature_it->value().size() < 5)
    {
        ctx.res_status = beast::http::status::forbidden;
        ctx.error = "Forbidden.";

        return handle_result::error;
    }

    hook_event ev = hook_event::unknown;
    if(event_it->value() == "push")
    {
        ev = hook_event::push;
    }
    else if(event_it->value() == "ping")
    {
        ev = hook_event::ping;
    }

    std::string signature{signature_it->value().substr(5, std::string::npos)};

    reader.async_read_body<beast::http::string_body>(
            [](beast::http::request<beast::http::string_body>& req){},
            [&, ctx, project, job, ev, signature, query_string](
                    beast::http::request<
                            beast::http::string_body>&& req,
                    const boost::beast::error_code& /*ec*/,
                    net::http_session::queue& queue) mutable
            {
                handle_github_signature(
                        std::move(req),
                        ctx,
                        queue,
                        project,
                        job,
                        query_string,
                        ev,
                        signature);
            });

    return handle_result::done;
}

handle_result webhooks_handler::handle_gitlab_headers(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        const std::string& project,
        const std::string& job,
        const std::string& query_string)
{
    auto signature_it = req.find("X-Gitlab-Token");
    auto event_it = req.find("X-Gitlab-Event");
    if(signature_it == req.end()
            || event_it == req.end()
            || signature_it->value() != ctx.api_access_key)
    {
        ctx.res_status = beast::http::status::forbidden;
        ctx.error = "Forbidden.";

        return handle_result::error;
    }

    hook_event ev = hook_event::unknown;
    if(event_it->value() == "Push Hook")
    {
        ev = hook_event::push;
    }

    reader.async_read_body<beast::http::string_body>(
            [](beast::http::request<beast::http::string_body>& req){},
            [&, ctx, project, job, ev, query_string](
                    beast::http::request<
                            beast::http::string_body>&& req,
                    const boost::beast::error_code& /*ec*/,
                    net::http_session::queue& queue) mutable
            {
                finish(
                        std::move(req),
                        ctx,
                        queue,
                        project,
                        job,
                        query_string,
                        ev);
            });

    return handle_result::done;
}

handle_result webhooks_handler::handle_plain_headers(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        const std::string& project,
        const std::string& job,
        const std::string& query_string)
{
    auto signature_it = req.find("X-Plain-Token");

    auto event_it = req.find("X-Plain-Event");

    if(signature_it == req.end()
            || event_it == req.end()
            || signature_it->value() != ctx.api_access_key)
    {
        ctx.res_status = beast::http::status::forbidden;
        ctx.error = "Forbidden.";

        return handle_result::error;
    }

    hook_event ev = hook_event::unknown;
    if(event_it->value() == "push")
    {
        ev = hook_event::push;
    }
    else if(event_it->value() == "ping")
    {
        ev = hook_event::ping;
    }

    reader.async_read_body<beast::http::string_body>(
            [](beast::http::request<beast::http::string_body>& req){},
            [&, ctx, project, job, ev, query_string](
                    beast::http::request<
                            beast::http::string_body>&& req,
                    const boost::beast::error_code& /*ec*/,
                    net::http_session::queue& queue) mutable
            {
                finish(
                        std::move(req),
                        ctx,
                        queue,
                        project,
                        job,
                        query_string,
                        ev);
            });

    return handle_result::done;
}


void webhooks_handler::handle_github_signature(
        beast::http::request<beast::http::string_body>&& req,
        request_context& ctx,
        net::http_session::queue& queue,
        const std::string& project,
        const std::string& job,
        const std::string& query_string,
        hook_event ev,
        const std::string& signature)
{
    openssl::hmac hmac;

    hmac.set_secret_key(
            reinterpret_cast<unsigned char*>(ctx.api_access_key.data()),
            ctx.api_access_key.length());

    auto result = hmac.calc_digest(
            reinterpret_cast<unsigned char*>(req.body().data()),
            req.body().length());

    std::stringstream hex_hmac_ss;
    for(auto val : result)
    {
        hex_hmac_ss << std::setfill('0') << std::setw(2) << std::hex
                    << static_cast<int>(val);
    }

    auto hex_hmac = hex_hmac_ss.str();

    if(hex_hmac != signature)
    {
        beast::http::response<beast::http::string_body> res{
                beast::http::status::forbidden,
                req.version()};
                res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(beast::http::field::content_type, "text/html");
                res.keep_alive(req.keep_alive());
        res.body() = "Forbidden.";
        queue.send(std::move(res));
    }

    finish(std::move(req), ctx, queue, project, job, query_string, ev);
}

void webhooks_handler::finish(
        beast::http::request<beast::http::string_body>&& req,
        request_context& ctx,
        net::http_session::queue& queue,
        const std::string& project,
        const std::string& job,
        const std::string& query_string,
        hook_event ev)
{
    if(ev != hook_event::ping)
    {
        auto file_path = save_body(req.body());

        auto params = prepare_params(
                project,
                job,
                query_string,
                file_path,
                ev);

        make_async_chain(boost::asio::system_executor{})
            .then(cis_.run_job(
                        project,
                        job,
                        params,
                        {},
                        {}))
            .then([file_path](auto&&...)
                        {
                            std::error_code ec;

                            std::filesystem::remove_all(file_path, ec);
                        })
            .run();
    }

    beast::http::response<beast::http::empty_body> res{
            beast::http::status::ok,
            req.version()};
            res.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(beast::http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());

    queue.send(std::move(res));
}

std::filesystem::path webhooks_handler::save_body(std::string_view body)
{
    std::filesystem::path result = std::filesystem::current_path();
    result = result / "webhooks_temp";

    while(true)
    {
        uint64_t name_str = random_generator::instance()();
        std::stringstream ss;
        ss << std::hex << name_str;
        auto path = result / ss.str();

        if(!std::filesystem::exists(path))
        {
            result = path;

            break;
        }
    }

    std::ofstream file(result, std::ofstream::binary);
    file << body;

    return result;
}

const char* webhooks_handler::ev_to_string(hook_event ev)
{
    switch(ev)
    {
        case hook_event::push:
        {
            return "push";
        }
        case hook_event::ping:
        {
            return "ping";
        }
        case hook_event::unknown:
        [[fallthrough]];
        default:
        {
            return "unknown";
        }
    }
}

std::vector<std::string> webhooks_handler::prepare_params(
        const std::string& project,
        const std::string& job,
        const std::string& query_string,
        const std::filesystem::path& body_file,
        hook_event ev)
{
    std::vector<std::string> result;

    auto job_info = cis_.get_job_info(project, job);
    if(job_info == nullptr)
    {
        return result;
    }
    auto params = job_info->get_params();

    result.resize(params.size());

    auto query_params = parse_request_query(query_string);

    size_t i = 0;
    for(auto& param : params)
    {
        if(auto it = query_params.find(param.name);
                it != query_params.end())
        {
            result[i] = it->second;
        }
        else if(param.name == "webhook_query_string")
        {
            result[i] = query_string;
        }
        else if(param.name == "webhook_request_body")
        {
            result[i] = body_file.string();
        }
        else if(param.name == "webhook_event_type")
        {
            result[i] = ev_to_string(ev);
        }
        ++i;
    }

    return result;
}

} // namespace http
