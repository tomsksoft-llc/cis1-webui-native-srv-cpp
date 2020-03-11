/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"
#include "auth_manager.h"
#include "rights_manager.h"
#include "cis/cis_manager.h"

namespace beast = boost::beast;

namespace http
{

class webhooks_handler
{
public:
    enum class api
    {
        github,
        gitlab,
        plain,
    };

    enum class hook_event
    {
        ping,
        push,
        tag_push,
        issue,
        note,
        merge_request,
        wiki_page,
        pipeline,
        build,
        unknown,
    };

    webhooks_handler(
            auth_manager& auth,
            rights_manager& rights,
            cis::cis_manager_interface& cis);

    handle_result operator()(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& email,
            const std::string& project,
            const std::string& job,
            const std::string& escaped_query_string,
            api api_provider);
private:
    auth_manager& auth_;
    rights_manager& rights_;
    cis::cis_manager_interface& cis_;

    handle_result handle_github_headers(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& job,
            const std::string& query_string);

    handle_result handle_gitlab_headers(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& job,
            const std::string& query_string);

    handle_result handle_plain_headers(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& job,
            const std::string& query_string);

    void handle_github_signature(
            beast::http::request<beast::http::string_body>&& req,
            request_context& ctx,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& job,
            const std::string& query_string,
            const std::string& raw_event,
            hook_event ev,
            const std::string& signature);

    void finish(
            beast::http::request<beast::http::string_body>&& req,
            request_context& ctx,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& job,
            const std::string& query_string,
            const std::string& raw_event,
            hook_event ev);

    std::filesystem::path save_body(std::string_view body);

    const char* ev_to_string(hook_event ev);

    std::vector<std::pair<std::string, std::string>> prepare_params(
            const std::string& project,
            const std::string& job,
            const std::string& query_string,
            const std::filesystem::path& body_file,
            const std::string& raw_event,
            hook_event ev);
};

} // namespace http
