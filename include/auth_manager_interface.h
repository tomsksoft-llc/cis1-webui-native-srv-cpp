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
    std::string name;
    std::string email;
    std::string group;
    std::optional<std::string> api_access_key;
};

struct auth_manager_interface
{
    virtual ~auth_manager_interface() = default;

    virtual std::optional<std::string> authenticate(
            const std::string& username,
            const std::string& pass,
            std::error_code& ec) = 0;

    virtual std::optional<std::string> authenticate(
            const std::string& token,
            std::error_code& ec) = 0;

    virtual bool has_user(
            const std::string& username,
            std::error_code& ec) const = 0;

    virtual bool has_email(
            const std::string& email,
            std::error_code& ec) const = 0;

    virtual bool change_group(
            const std::string& username,
            const std::string& groupname,
            std::error_code& ec) = 0;

    virtual std::optional<std::string> get_group(
            const std::string& username,
            std::error_code& ec) const = 0;

    virtual std::optional<std::string> generate_api_key(
            const std::string& name,
            std::error_code& ec) = 0;

    virtual std::optional<std::string> get_api_key(
            const std::string& name,
            std::error_code& ec) = 0;

    virtual bool remove_api_key(
            const std::string& name,
            std::error_code& ec) = 0;

    virtual bool change_pass(
            const std::string& user,
            const std::string& old_pass,
            const std::string& new_pass,
            std::error_code& ec) = 0;

    virtual std::optional<user_info> get_user_info(
            const std::string& username,
            std::error_code& ec) const = 0;

    virtual std::vector<database::user> get_users(
            std::error_code& ec) const = 0;

    virtual std::vector<user_info> get_user_infos(
            std::error_code& ec) const = 0;

    virtual std::optional<database::group> get_group_info(
            const std::string& group_name,
            std::error_code& ec) const = 0;

    virtual bool delete_token(
            const std::string& token,
            std::error_code& ec) = 0;

    virtual bool add_user(
            const std::string& username,
            const std::string& pass,
            const std::string& email,
            std::error_code& ec) = 0;
};
