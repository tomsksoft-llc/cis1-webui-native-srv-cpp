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
private:
    boost::asio::io_context& ioc_;
    database::database_wrapper& db_;
};
