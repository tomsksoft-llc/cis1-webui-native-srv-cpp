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
        gitlab
    };

    webhooks_handler(
            auth_manager& auth,
            rights_manager& rights,
            cis::cis_manager& cis);

    handle_result operator()(
            beast::http::request<beast::http::empty_body>& req,
            request_context& ctx,
            net::http_session::request_reader& reader,
            net::http_session::queue& queue,
            const std::string& username,
            const std::string& project,
            const std::string& job,
            const std::string& escaped_query_string,
            api api_provider);
private:
    enum class hook_event
    {
        push,
        unknown
    };

    auth_manager& auth_;
    rights_manager& rights_;
    cis::cis_manager& cis_;

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

    void handle_github_signature(
            beast::http::request<beast::http::string_body>&& req,
            request_context& ctx,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& job,
            const std::string& query_string,
            hook_event ev,
            const std::string& signature);

    void finish(
            beast::http::request<beast::http::string_body>&& req,
            request_context& ctx,
            net::http_session::queue& queue,
            const std::string& project,
            const std::string& job,
            const std::string& query_string,
            hook_event ev);
    
    std::filesystem::path save_body(std::string_view body);

    const char* ev_to_string(hook_event ev);

    std::vector<std::string> prepare_params(
            const std::string& project,
            const std::string& job,
            const std::string& query_string,
            const std::filesystem::path& body_file,
            hook_event ev);
};

} // namespace http
