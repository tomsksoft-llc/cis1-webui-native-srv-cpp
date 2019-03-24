#include "rights_manager.h"

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#include "file_util.h"
#include "dirs.h"

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

std::optional<bool> rights_manager::check_right(
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
    //TODO document.HasParseErrors() ...
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
}
