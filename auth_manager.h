#pragma once

#include <filesystem>
#include <string>
#include <map>
#include <optional>

#include "net/http_session.h"
#include "handle_result.h"
#include "request_context.h"

struct user
{
    std::string pass;
    std::string email;
    bool admin;
    bool disabled;
    std::optional<std::string> api_access_key;
};

class auth_manager
{
    std::map<std::string, user> users_;
    std::map<std::string, std::string> tokens_;
public:
    auth_manager();
    std::string authenticate(const std::string& user, const std::string& pass);
    std::string authenticate(const std::string& token);
    bool has_user(const std::string& name);
    void set_disabled(const std::string& name, bool state);
    void make_admin(const std::string& name, bool state);
    bool change_pass(
            const std::string& user,
            const std::string& old_pass,
            const std::string& new_pass);
    const std::map<std::string, user>& get_users();
    void delete_token(const std::string& token);
    std::optional<std::string> add_user(std::string user, std::string pass);
    void save_on_disk();
private:
    void load_users(std::filesystem::path users_file);
    void load_tokens(std::filesystem::path tokens_file);
    void save_users(std::filesystem::path users_file);
    void save_tokens(std::filesystem::path tokens_file);
};
