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

#include "net/http_session.h"
#include "http/handle_result.h"
#include "request_context.h"
#include "database_structures.h"

struct user_info
{
    std::string email;
    std::optional<std::string> api_access_key;
    bool is_admin;
};

struct auth_manager_interface
{
    virtual ~auth_manager_interface() = default;

    virtual std::optional<std::string> authenticate(
            const std::string& email,
            const std::string& pass,
            std::error_code& ec) = 0;

    virtual std::optional<std::string> authenticate(
            const std::string& token,
            std::error_code& ec) = 0;

    virtual bool has_user(
            const std::string& email,
            std::error_code& ec) const = 0;

    virtual std::optional<std::string> generate_api_key(
            const std::string& email,
            std::error_code& ec) = 0;

    virtual std::optional<std::string> get_api_key(
            const std::string& email,
            std::error_code& ec) = 0;

    virtual bool remove_api_key(
            const std::string& email,
            std::error_code& ec) = 0;

    virtual bool change_pass(
            const std::string& email,
            const std::string& old_pass,
            const std::string& new_pass,
            std::error_code& ec) = 0;

    virtual std::optional<user_info> get_user_info(
            const std::string& email,
            std::error_code& ec) const = 0;

    virtual std::vector<database::user> get_users(
            std::error_code& ec) const = 0;

    virtual std::vector<user_info> get_user_infos(
            std::error_code& ec) const = 0;

    virtual bool delete_token(
            const std::string& token,
            std::error_code& ec) = 0;

    virtual bool add_user(
            const std::string& email,
            const std::string& pass,
            bool admin,
            std::error_code& ec) = 0;

    virtual bool delete_user(
            const std::string& email,
            std::error_code& ec) = 0;
};
