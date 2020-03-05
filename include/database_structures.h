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

struct user
{
    intmax_t id;
    std::string email;
    std::string pass;
    bool admin;
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

} // namespace database
