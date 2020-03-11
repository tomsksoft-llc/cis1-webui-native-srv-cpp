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

    bool is_admin(
            const std::string& email,
            std::error_code& ec) const final;

    std::optional<database::project_user_right> check_project_right(
            const std::string& email,
            const std::string& project,
            std::error_code& ec) const final;

    std::map<std::string, project_rights> get_permissions(
            const std::string& email,
            std::error_code& ec) const final;

    bool set_user_project_permissions(
            const std::string& email,
            const std::string& project,
            database::project_user_right rights,
            std::error_code& ec) final;

private:
    boost::asio::io_context& ioc_;
    database::database_wrapper& db_;
};
