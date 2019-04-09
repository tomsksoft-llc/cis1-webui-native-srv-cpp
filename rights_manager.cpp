#include "rights_manager.h"

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include "file_util.h"
#include "db_dirs.h"
#include "load_config_error.h"

constexpr const char* rights_file_path = "/rights.pwd";

rights_manager::rights_manager()
{
    load_rights(path_cat(db::get_root_dir(), rights_file_path));
}

void rights_manager::add_resource(
        const std::string& resource_name,
        bool default_value)
{
    resources_.try_emplace(resource_name, default_value);
    save_rights(path_cat(db::get_root_dir(), rights_file_path));
}

void rights_manager::set_right(
        const std::string& username,
        const std::string& resource_name,
        bool value)
{
    user_rights_[username][resource_name] = value;
    save_rights(path_cat(db::get_root_dir(), rights_file_path));
}

std::optional<bool> rights_manager::check_user_right(
        const std::string& username,
        const std::string& resource_name)
{
    if(auto user = user_rights_.find(username); user != user_rights_.end())
    {
        if(auto resource = user->second.find(resource_name);
                resource != user->second.end())
        {
            return resource->second;
        }
    }
    if(auto resource = resources_.find(resource_name); resource != resources_.end())
    {
        return resource->second;
    }
    return std::nullopt;
}

std::optional<project_rights> rights_manager::check_project_right(
        const std::string& username,
        const std::string& project)
{
    if(auto user = projects_permissions_.find(username); user != projects_permissions_.end())
    {
        if(auto resource = user->second.find(project);
                resource != user->second.end())
        {
            return resource->second;
        }
    }
    return std::nullopt;
}

const std::map<std::string, project_rights>& rights_manager::get_permissions(const std::string& username) const
{
    return projects_permissions_.at(username);
}

void rights_manager::set_user_project_permissions(const std::string& user, const std::string& project, project_rights rights)
{
    projects_permissions_[user][project] = rights;
}

void rights_manager::save_rights(const std::filesystem::path& file)
{
    rapidjson::Document document;
    rapidjson::Value document_value;
    document.SetObject();
    document_value.SetObject();
    rapidjson::Value key;
    rapidjson::Value value;
    for(auto& [resource, right] : resources_)
    {
        key.SetString(resource.c_str(), resource.length(), document.GetAllocator());
        value.SetBool(right);
        document_value.AddMember(key, value, document.GetAllocator());
    }
    document.AddMember("resources", document_value, document.GetAllocator());
    
    document_value.SetObject();
    for(auto& [user, rights] : user_rights_)
    {
        key.SetString(user.c_str(), user.length(), document.GetAllocator());
        value.SetObject();
        rapidjson::Value second_key;
        rapidjson::Value second_value;
        for(auto& [resource, right] : rights)
        {
            second_key.SetString(
                    resource.c_str(),
                    resource.length(),
                    document.GetAllocator());
            second_value.SetBool(right);
            value.AddMember(second_key, second_value, document.GetAllocator());
        }
        document_value.AddMember(key, value, document.GetAllocator());
    }
    document.AddMember("user_rights", document_value, document.GetAllocator());

    document_value.SetObject();
    for(auto& [user, projects] : projects_permissions_)
    {
        key.SetString(user.c_str(), user.length(), document.GetAllocator());
        value.SetObject();
        rapidjson::Value second_key;
        rapidjson::Value second_value;
        for(auto& [project, rights] : projects)
        {
            second_key.SetString(
                    project.c_str(),
                    project.length(),
                    document.GetAllocator());
            second_value.SetObject();
            second_value.AddMember(
                    "read",
                    rapidjson::Value().SetBool(rights.read),
                    document.GetAllocator());
            second_value.AddMember(
                    "write",
                    rapidjson::Value().SetBool(rights.write),
                    document.GetAllocator());
            second_value.AddMember(
                    "execute",
                    rapidjson::Value().SetBool(rights.execute),
                    document.GetAllocator());
            value.AddMember(second_key, second_value, document.GetAllocator());
        }
        document_value.AddMember(key, value, document.GetAllocator());
    }
    document.AddMember("projects_permissions", document_value, document.GetAllocator());

    std::ofstream db(file);
    rapidjson::OStreamWrapper osw(db);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    document.Accept(writer);
}

void rights_manager::load_rights(const std::filesystem::path& file)
{
    std::ifstream db(file);
    rapidjson::IStreamWrapper isw(db);
    rapidjson::Document document;
    document.ParseStream(isw);
    if(document.HasParseError())
    {
        throw load_config_error("Can't load user rights");
    }
    if(!document.IsObject())
    {
        return;
    }

    auto resources_obj = document["resources"].GetObject();
    for(auto& member : resources_obj)
    {
        resources_[member.name.GetString()] = member.value.GetBool();
    }

    auto user_rights_obj = document["user_rights"].GetObject();
    for(auto& member : user_rights_obj)
    {
        auto user = member.name.GetString();
        auto rights = member.value.GetObject();
        for(auto& resource_right : rights)
        {
            user_rights_[user][resource_right.name.GetString()]
                = resource_right.value.GetBool();
        }
    }

    auto projects_permissions_obj = document["projects_permissions"].GetObject();
    for(auto& member : projects_permissions_obj)
    {
        auto user = member.name.GetString();
        auto rights = member.value.GetObject();
        for(auto& resource_right : rights)
        {
            auto project_rights_obj = resource_right.value.GetObject();
            project_rights pr{
                project_rights_obj["read"].GetBool(),
                project_rights_obj["write"].GetBool(),
                project_rights_obj["execute"].GetBool()};
            projects_permissions_[user][resource_right.name.GetString()] = pr;
        }
    }
}
