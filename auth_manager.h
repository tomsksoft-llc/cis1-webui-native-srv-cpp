#pragma once

#include <filesystem>

enum class RIGHT
{
    write,
    read,
    exec
};

class auth_manager
{
    std::filesystem::path db_path_; 
public:
    auth_manager(std::string path);
    bool authorize(std::string resource, std::string user, RIGHT r);
    // "/projects" "enjection" RIGHT:read
    // "3" "enjection" RIGHT:read
};
