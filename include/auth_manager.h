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
            const std::string& pass);

    std::optional<std::string> authenticate(
            const std::string& token);

    bool has_user(
            const std::string& username) const;

    bool change_group(
            const std::string& username,
            const std::string& groupname);

    std::optional<std::string> get_group(
            const std::string& username) const;

    std::optional<std::string> generate_api_key(
            const std::string& name);

    std::optional<std::string> get_api_key(
            const std::string& name);

    bool remove_api_key(
            const std::string& name);

    bool change_pass(
            const std::string& user,
            const std::string& old_pass,
            const std::string& new_pass);

    std::optional<user_info> get_user_info(
            const std::string& username) const;

    std::vector<database::user> get_users() const;

    std::vector<user_info> get_user_infos() const;

    bool delete_token(
            const std::string& token);

    bool add_user(
            const std::string& username,
            const std::string& pass,
            const std::string& email);

    void cleanup();

private:
    database::database_wrapper& db_;
    boost::asio::steady_timer timer_;
};
