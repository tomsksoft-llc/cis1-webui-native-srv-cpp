/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <string>
#include <map>
#include <optional>
#include <vector>
#include <filesystem>

#include "rights_manager_interface.h"
#include "database_structures.h"
#include "request_context.h"

struct project_rights
{
    bool read;
    bool write;
    bool execute;
};

struct rights_manager_interface
{
    virtual ~rights_manager_interface() = default;

    virtual bool is_admin(
            const std::string& email,
            std::error_code& ec) const = 0;

    virtual std::optional<database::project_user_right> check_project_right(
            const std::string& email,
            const std::string& project,
            std::error_code& ec) const = 0;

    virtual std::map<std::string, project_rights> get_projects_permissions(
            const std::string& username,
            std::error_code& ec) const = 0;

    virtual bool set_user_project_permissions(
            const std::string& user,
            const std::string& project,
            database::project_user_right rights,
            std::error_code& ec) = 0;

    virtual std::vector<std::string> get_user_permissions(
            const std::string& username,
            std::error_code& ec) const = 0;

    virtual bool set_group_default_permissions(
            intmax_t group_id,
            const project_rights& rights,
            std::error_code& ec) const = 0;
};
