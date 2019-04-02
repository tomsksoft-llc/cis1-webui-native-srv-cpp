#pragma once

#include <string>
#include <map>
#include <optional>
#include <filesystem>

struct project_rights
{
    bool read;
    bool write;
    bool execute;
};

class rights_manager
{
    std::map<std::string, bool> resources_;
    std::map<std::string, std::map<std::string, project_rights>> projects_permissions_;
    std::map<std::string, std::map<std::string, bool>> user_rights_;
public:
    rights_manager();
    void add_resource(
            const std::string& resource_name,
            bool default_value = false);
    void set_right(
            const std::string& username,
            const std::string& resource_name,
            bool value);
    std::optional<bool> check_user_right(
            const std::string& username,
            const std::string& resource_name);
    std::optional<project_rights> check_project_right(
            const std::string& username,
            const std::string& project);
    const std::map<std::string, project_rights>& get_permissions(const std::string& username) const;
    void set_user_project_permissions(const std::string& user, const std::string& project, project_rights rights);
    void save_rights(const std::filesystem::path& file);
    void load_rights(const std::filesystem::path& file);
};
