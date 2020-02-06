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

#include <boost/asio.hpp>

#include "rights_manager_interface.h"
#include "database.h"

class rights_manager
    : public rights_manager_interface
{
public:
    rights_manager(
            boost::asio::io_context& ioc,
            database::database_wrapper& db);

    std::optional<bool> check_user_permission(
            const request_context::cln_info_holder& cln_info,
            const std::string& permission_name,
            std::error_code& ec) const final;

    std::optional<database::project_user_right> get_project_user_right(
            const std::string& username,
            const std::string& project,
            std::error_code& ec) const final;

    std::optional<project_rights> check_project_right(
            const request_context::cln_info_holder& cln_info,
            const std::string& projectname,
            std::error_code& ec) const final;

    std::map<std::string, project_rights> get_projects_permissions(
            const std::string& username,
            std::error_code& ec) const final;

    bool set_user_project_permissions(
            const std::string& user,
            const std::string& project,
            database::project_user_right rights,
            std::error_code& ec) final;

    std::optional<database::group_default_rights> get_group_default_permissions(
            intmax_t group_id,
            std::error_code& ec) const final;

    bool set_group_default_permissions(
            intmax_t group_id,
            const project_rights& rights,
            std::error_code& ec) const final;

private:
    boost::asio::io_context& ioc_;
    database::database_wrapper& db_;
};
