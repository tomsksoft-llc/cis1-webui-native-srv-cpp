#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <optional>

#include "net/http_session.h"
#include "http/handle_result.h"
#include "request_context.h"
#include "database.h"

class auth_manager
{
    database::database& db_;
public:
    auth_manager(database::database& db);
    std::optional<std::string> authenticate(
            const std::string& username,
            const std::string& pass);
    std::optional<std::string> authenticate(
            const std::string& token);
    bool has_user(
            const std::string& username) const;
    bool change_group(
            const std::string& username,
            const std::string& groupname);
    std::optional<std::string> get_group(
            const std::string& username) const;
    std::optional<std::string> generate_api_key(
            const std::string& name);
    bool change_pass(
            const std::string& user,
            const std::string& old_pass,
            const std::string& new_pass);
    std::vector<database::user> get_users() const;
    bool delete_token(
            const std::string& token);
    bool add_user(
            std::string username,
            std::string pass,
            std::string email);
};
