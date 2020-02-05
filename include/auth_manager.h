/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <optional>

#include <boost/asio.hpp>

#include "auth_manager_interface.h"
#include "net/http_session.h"
#include "http/handle_result.h"
#include "request_context.h"
#include "database.h"

class auth_manager
    : public auth_manager_interface
{
public:
    auth_manager(
            boost::asio::io_context& ioc,
            database::database_wrapper& db);

    std::optional<std::string> authenticate(
            const std::string& username,
            const std::string& pass,
            std::error_code& ec) final;

    std::optional<std::string> authenticate(
            const std::string& token,
            std::error_code& ec) final;

    bool has_user(
            const std::string& username,
            std::error_code& ec) const final;

    bool has_email(
            const std::string& email,
            std::error_code& ec) const final;

    bool change_group(
            const std::string& username,
            const std::string& groupname,
            std::error_code& ec) final;

    std::optional<std::string> get_group(
            const std::string& username,
            std::error_code& ec) const final;

    std::optional<std::string> generate_api_key(
            const std::string& name,
            std::error_code& ec) final;

    std::optional<std::string> get_api_key(
            const std::string& name,
            std::error_code& ec) final;

    bool remove_api_key(
            const std::string& name,
            std::error_code& ec) final;

    bool change_pass(
            const std::string& user,
            const std::string& old_pass,
            const std::string& new_pass,
            std::error_code& ec) final;

    std::optional<user_info> get_user_info(
            const std::string& username,
            std::error_code& ec) const final;

    std::vector<database::user> get_users(
            std::error_code& ec) const final;

    std::vector<user_info> get_user_infos(
            std::error_code& ec) const final;

    std::optional<database::group> get_group_info(
            const std::string& group_name,
            std::error_code& ec) const final;

    bool delete_token(
            const std::string& token,
            std::error_code& ec) final;

    bool add_user(
            const std::string& username,
            const std::string& pass,
            const std::string& email,
            std::error_code& ec) final;

    void cleanup();

private:
    database::database_wrapper& db_;
    boost::asio::steady_timer timer_;
};
