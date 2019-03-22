#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <optional>

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"
#include "web_app.h"

class auth_manager
{
    std::map<std::string, std::string> users_;
    std::map<std::string, std::string> tokens_;
public:
    auth_manager();
    handle_result operator()(
            http::request<http::string_body>& req,
            http_session::queue& queue,
            request_context& ctx);
    std::string authenticate(const std::string& user, const std::string& pass);
    std::string authenticate(const std::string& token);
    void delete_token(const std::string& token);
    std::optional<std::string> add_user(std::string user, std::string pass);
    void save_on_disk();
private:
    void load_users(std::filesystem::path users_file);
    void load_tokens(std::filesystem::path tokens_file);
    void save_users(std::filesystem::path users_file);
    void save_tokens(std::filesystem::path tokens_file);
};
