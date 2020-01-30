/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <cstdint>
#include <string>

namespace database
{

struct group
{
    intmax_t id;
    std::string name;
};

struct permission
{
    intmax_t id;
    std::string name;
};

struct group_permission
{
    intmax_t id;
    intmax_t group_id;
    intmax_t permission_id;
};

struct user
{
    intmax_t id;
    intmax_t group_id;
    std::string name;
    std::string email;
    std::string pass;
};

struct project
{
    intmax_t id;
    std::string name;
    bool deleted;
};

struct token
{
    intmax_t id;
    intmax_t user_id;
    std::string value;
    uint64_t expiration_time;
};

struct api_access_key
{
    intmax_t id;
    intmax_t user_id;
    std::string value;
};

struct project_user_right
{
    intmax_t id;
    intmax_t project_id;
    intmax_t user_id;
    bool read;
    bool write;
    bool execute;
};

struct projects_group_right
{
    intmax_t id;
    intmax_t group_id;
    bool read;
    bool write;
    bool execute;
};

} // namespace database
