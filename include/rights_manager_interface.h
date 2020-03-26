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
    bool read = false;
    bool write = false;
    bool execute = false;
};

struct project_rights_ex
{
    bool admin = false;
    bool read = false;
    bool write = false;
    bool execute = false;
};

struct rights_manager_interface
{
    virtual ~rights_manager_interface() = default;

    virtual bool is_admin(
            const std::string& email,
            std::error_code& ec) const = 0;

    virtual bool set_admin_status(
            const std::string& email,
            bool admin,
            std::error_code& ec) const = 0;

    virtual std::optional<project_rights> check_project_right(
            const std::string& email,
            const std::string& project,
            std::error_code& ec) const = 0;

    virtual std::map<std::string, project_rights> get_permissions(
            const std::string& email,
            std::error_code& ec) const = 0;

    virtual std::map<std::string, project_rights_ex> get_permissions_by_project(
            const std::string& project,
            std::error_code& ec) const = 0;

    virtual bool set_user_project_permissions(
            const std::string& email,
            const std::string& project,
            database::project_user_right rights,
            std::error_code& ec) = 0;
};
