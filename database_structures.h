#pragma once

#include <string>

namespace database
{

struct group
{
    ssize_t id;
    std::string name;
};

struct permission
{
    ssize_t id;
    std::string name;
};

struct group_permission
{
    ssize_t id;
    ssize_t group_id;
    ssize_t permission_id;
};

struct user
{
    ssize_t id;
    ssize_t group_id;
    std::string name;
    std::string email;
    std::string pass;
};

struct project
{
    ssize_t id;
    std::string name;
    bool deleted;
};

struct token
{
    ssize_t id;
    ssize_t user_id;
    std::string value;
    uint64_t expiration_time;
};

struct api_access_key
{
    ssize_t id;
    ssize_t user_id;
    std::string value;
};

struct project_user_right
{
    ssize_t id;
    ssize_t project_id;
    ssize_t user_id;
    bool read;
    bool write;
    bool execute;
};

} // namespace database
