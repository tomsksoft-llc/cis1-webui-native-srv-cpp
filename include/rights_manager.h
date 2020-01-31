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
            const std::string& username,
            const std::string& permission_name,
            std::error_code& ec) const override;

    std::optional<database::project_user_right> check_project_right(
            const std::string& username,
            const std::string& project,
            std::error_code& ec) const override;

    std::map<std::string, project_rights> get_permissions(
            const std::string& username,
            std::error_code& ec) const override;

    bool set_user_project_permissions(
            const std::string& user,
            const std::string& project,
            database::project_user_right rights,
            std::error_code& ec) override;

    bool set_group_projects_permissions(
            intmax_t group_id,
            const project_rights& rights,
            std::error_code& ec);

private:
    boost::asio::io_context& ioc_;
    database::database_wrapper& db_;
};
