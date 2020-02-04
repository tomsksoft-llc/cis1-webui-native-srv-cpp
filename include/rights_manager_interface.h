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

    virtual std::optional<bool> check_user_permission(
            const request_context::cln_info_holder& cln_info,
            const std::string& permission_name,
            std::error_code& ec) const = 0;

    virtual std::optional<database::project_user_right> get_project_user_right(
            const std::string& username,
            const std::string& project,
            std::error_code& ec) const = 0;

    virtual std::optional<project_rights> check_project_right(
            const request_context::cln_info_holder& cln_info,
            const std::string& project,
            std::error_code& ec) const = 0;

    virtual std::map<std::string, project_rights> get_permissions(
            const std::string& username,
            std::error_code& ec) const = 0;

    virtual bool set_user_project_permissions(
            const std::string& user,
            const std::string& project,
            database::project_user_right rights,
            std::error_code& ec) = 0;

    virtual std::optional<database::projects_group_right> get_group_projects_permissions(
            intmax_t group_id,
            std::error_code& ec) const = 0;

    virtual bool set_group_projects_permissions(
            intmax_t group_id,
            const project_rights& rights,
            std::error_code& ec) const = 0;
};
