#pragma once

#include <filesystem>

//TODO: remove?
class auth_manager
{
    std::filesystem::path db_path_; 
public:
    auth_manager(std::string path);
    bool authorize(std::string resource, std::string user, RIGHT r);
};
