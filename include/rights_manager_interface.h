#pragma once

#include <string>
#include <map>
#include <optional>
#include <filesystem>

#include "rights_manager_interface.h"
#include "database_structures.h"

struct project_rights
{
    bool read;
    bool write;
    bool execute;
};

struct rights_manager_interface
{
    virtual ~rights_manager_interface() = default;

    virtual std::optional<bool> check_user_permission(
            const std::string& username,
            const std::string& permission_name) const = 0;

    virtual std::optional<database::project_user_right> check_project_right(
            const std::string& username,
            const std::string& project) const = 0;

    virtual std::map<std::string, project_rights> get_permissions(
            const std::string& username) const = 0;

    virtual bool set_user_project_permissions(
            const std::string& user,
            const std::string& project,
            database::project_user_right rights) = 0;
};
