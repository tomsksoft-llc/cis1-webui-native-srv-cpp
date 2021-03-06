/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "http/webhooks_handler.h"

#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "openssl_wrapper/openssl_wrapper.h"
#include "http/request_util.h"
#include "random_generator.h"

namespace http
{

const std::map<std::string, webhooks_handler::hook_event, std::less<>> github_mapping =
{
    {"ping", webhooks_handler::hook_event::ping},
    {"push", webhooks_handler::hook_event::push},
    {"issues", webhooks_handler::hook_event::issue},
    {"pull_request", webhooks_handler::hook_event::merge_request},
    {"gollum", webhooks_handler::hook_event::wiki_page},
};

const std::map<std::string, webhooks_handler::hook_event, std::less<>> gitlab_mapping =
{
    {"Push Hook", webhooks_handler::hook_event::push},
    {"Tag Push Hook", webhooks_handler::hook_event::tag_push},
    {"Issue Hook", webhooks_handler::hook_event::issue},
    {"Note Hook", webhooks_handler::hook_event::note},
    {"Merge Request Hook", webhooks_handler::hook_event::merge_request},
    {"Wiki Page Hook", webhooks_handler::hook_event::wiki_page},
    {"Pipeline Hook", webhooks_handler::hook_event::pipeline},
    {"Job Hook", webhooks_handler::hook_event::build},
};

const std::map<std::string, webhooks_handler::hook_event, std::less<>> plain_mapping =
{
    {"ping", webhooks_handler::hook_event::ping},
    {"push", webhooks_handler::hook_event::push},
    {"tag_push", webhooks_handler::hook_event::tag_push},
    {"issue", webhooks_handler::hook_event::issue},
    {"note", webhooks_handler::hook_event::note},
    {"merge_request", webhooks_handler::hook_event::merge_request},
    {"wiki_page", webhooks_handler::hook_event::wiki_page},
    {"pipeline", webhooks_handler::hook_event::pipeline},
    {"build", webhooks_handler::hook_event::build},
};

webhooks_handler::webhooks_handler(
        auth_manager& auth,
        rights_manager& rights,
        cis::cis_manager_interface& cis)
    : auth_(auth)
    , rights_(rights)
    , cis_(cis)
{}

handle_result webhooks_handler::operator()(
        beast::http::request<beast::http::empty_body>& req,
        request_context& ctx,
        net::http_session::request_reader& reader,
        net::http_session::queue& queue,
        const std::string& email,
        const std::string& project,
        const std::string& job,
        const std::string& escaped_query_string,
        api api_provider)
{
    if(req.method() != beast::http::verb::post)
    {
        ctx.allowed_verbs = {
            beast::http::verb::post
        };
        ctx.res_status = beast::http::status::method_not_allowed;

        return handle_result::error;
    }

    std::error_code ec;

    auto user = auth_.get_user_info(email, ec);

    if(ec)
    {
        ctx.res_status = beast::http::status::internal_server_error;

        return http::handle_result::error;
    }

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

    const std::string active_token
            = ctx.client_info
              ? ctx.client_info.value().active_token
              : std::string{};

    ctx.client_info = request_context::user_info{email,
                                                 active_token,
                                                 user->api_access_key.value()};

    auto project_rights
            = rights_.check_project_right(email, project, ec);

    if(ec)
    {
        ctx.res_status = beast::http::status::internal_server_error;

        return handle_result::error;
    }

    if(!project_rights || !project_rights.value().write)
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
    if(auto mapping_it = github_mapping.find(event_it->value());
            mapping_it != github_mapping.end())
    {
        ev = mapping_it->second;
    }

    std::string signature{signature_it->value().substr(5, std::string::npos)};

    reader.async_read_body<beast::http::string_body>(
            [](beast::http::request<beast::http::string_body>& req){},
            [&,
            ctx,
            project,
            job,
            raw_event = std::string(event_it->value()),
            ev,
            signature,
            query_string](
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
                        raw_event,
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
    const std::string api_access_key
            = ctx.client_info
              ? ctx.client_info.value().api_access_key
              : std::string{};

    auto signature_it = req.find("X-Gitlab-Token");
    auto event_it = req.find("X-Gitlab-Event");
    if(signature_it == req.end()
            || event_it == req.end()
            || signature_it->value() != api_access_key)
    {
        ctx.res_status = beast::http::status::forbidden;
        ctx.error = "Forbidden.";

        return handle_result::error;
    }

    hook_event ev = hook_event::unknown;
    if(auto mapping_it = gitlab_mapping.find(event_it->value());
            mapping_it != gitlab_mapping.end())
    {
        ev = mapping_it->second;
    }

    reader.async_read_body<beast::http::string_body>(
            [](beast::http::request<beast::http::string_body>& req){},
            [&,
            ctx,
            project,
            job,
            raw_event = std::string(event_it->value()),
            ev,
            query_string](
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
                        raw_event,
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
    const std::string api_access_key
            = ctx.client_info
              ? ctx.client_info.value().api_access_key
              : std::string{};

    auto signature_it = req.find("X-Plain-Token");
    auto event_it = req.find("X-Plain-Event");

    if(signature_it == req.end()
            || event_it == req.end()
            || signature_it->value() != api_access_key)
    {
        ctx.res_status = beast::http::status::forbidden;
        ctx.error = "Forbidden.";

        return handle_result::error;
    }

    hook_event ev = hook_event::unknown;
    if(auto mapping_it = plain_mapping.find(event_it->value());
            mapping_it != plain_mapping.end())
    {
        ev = mapping_it->second;
    }

    reader.async_read_body<beast::http::string_body>(
            [](beast::http::request<beast::http::string_body>& req){},
            [&,
            ctx,
            project,
            job,
            raw_event = std::string(event_it->value()),
            ev,
            query_string](
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
                        raw_event,
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
        const std::string& raw_event,
        hook_event ev,
        const std::string& signature)
{
    openssl::hmac hmac;

    assert(ctx.client_info);
    auto& cln_info = ctx.client_info.value();

    hmac.set_secret_key(
            reinterpret_cast<unsigned char*>(cln_info.api_access_key.data()),
            cln_info.api_access_key.length());

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

    finish(
            std::move(req),
            ctx,
            queue,
            project,
            job,
            query_string,
            raw_event,
            ev);
}

void webhooks_handler::finish(
        beast::http::request<beast::http::string_body>&& req,
        request_context& ctx,
        net::http_session::queue& queue,
        const std::string& project,
        const std::string& job,
        const std::string& query_string,
        const std::string& raw_event,
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
                raw_event,
                ev);

        assert(ctx.client_info);
        const auto& email = ctx.client_info.value().email;

        make_async_chain(queue.get_executor())
            .then(cis_.run_job(
                        project,
                        job,
                        true,
                        params,
                        {},
                        {},
                        email))
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
        case hook_event::ping:
        {
            return "ping";
        }
        case hook_event::push:
        {
            return "push";
        }
        case hook_event::tag_push:
        {
            return "tag_push";
        }
        case hook_event::issue:
        {
            return "issue";
        }
        case hook_event::note:
        {
            return "note";
        }
        case hook_event::merge_request:
        {
            return "merge_request";
        }
        case hook_event::wiki_page:
        {
            return "wiki_page";
        }
        case hook_event::pipeline:
        {
            return "pipeline";
        }
        case hook_event::build:
        {
            return "build";
        }
        case hook_event::unknown:
        [[fallthrough]];
        default:
        {
            return "unknown";
        }
    }
}

std::vector<std::pair<std::string, std::string>> webhooks_handler::prepare_params(
        const std::string& project,
        const std::string& job,
        const std::string& query_string,
        const std::filesystem::path& body_file,
        const std::string& raw_event,
        hook_event ev)
{
    std::vector<std::pair<std::string, std::string>> result;

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
            result[i] = {param.name, it->second};
        }
        else if(param.name == "webhook_query_string")
        {
            result[i] = {param.name, query_string};
        }
        else if(param.name == "webhook_request_body")
        {
            result[i] = {param.name, body_file.string()};
        }
        else if(param.name == "webhook_event_type")
        {
            result[i] = {param.name, ev_to_string(ev)};
        }
        else if(param.name == "webhook_raw_event_type")
        {
            result[i] = {param.name, raw_event};
        }
        ++i;
    }

    return result;
}

} // namespace http
