#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <optional>

#include "web_app.h"

class auth_manager
{
    std::map<std::string, std::string> users_;
    std::map<std::string, std::string> tokens_;
public:
    auth_manager();
    web_app::handle_result operator()(
            web_app::request_t& req,
            web_app::queue_t& queue,
            web_app::context_t& ctx);
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
