#pragma once

#include <string>
#include <map>
#include <optional>
#include <filesystem>

#include "database.h"

struct project_rights
{
    bool read;
    bool write;
    bool execute;
};

class rights_manager
{
    database::database& db_;
public:
    rights_manager(database::database& db);
    std::optional<bool> check_user_permission(
            const std::string& username,
            const std::string& permission_name) const;
    std::optional<database::project_user_right> check_project_right(
            const std::string& username,
            const std::string& project) const;
    std::map<std::string, project_rights> get_permissions(
            const std::string& username) const;
    bool set_user_project_permissions(
            const std::string& user,
            const std::string& project,
            database::project_user_right rights);
};
