#pragma once

#include <string>
#include <map>
#include <optional>
#include <filesystem>

class rights_manager
{
    std::map<std::string, bool> resources_;
    //user -> resource -> bool
    std::map<std::string, std::map<std::string, bool>> user_rights_;
public:
    // "projects.<project_name>.read"
    // "projects.<project_name>.write"
    // "projects.<project_name>.exec"
    // "admin"
    void add_resource(std::string resource_name, bool default_value = false);
    // "enjection" "projects.internal.read" true
    void set_right(std::string username, std::string resource_name, bool value);
    // "enjection" "projects.internal.read" -> true
    std::optional<bool> check_right(std::string username, std::string resource_name);

    void save_to_file(const std::filesystem::path& file);
    void load_from_file(const std::filesystem::path& file);
};
